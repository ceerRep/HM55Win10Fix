#include "io.h"
#include "util.h"

typedef struct
{
    char sig[4];
    u32 length;
    u8 rev;
    u8 checksum;
    char oemid[6];
    char unused[16];
    unsigned char data[];
} DESC_HEADER;

typedef struct
{
    char sig[8];
    char unused[8];
    DESC_HEADER *rsdt;
} RSDP;

static inline int memcmp(const void *a, const void *b, u32 len)
{
    for (; len; len--, a++, b++)
    {
        if (*(const char *)a != *(const char *)b)
            return *(const char *)a - *(const char *)b;
    }

    return 0;
}

static inline void memset(void *a, u32 len, u8 val)
{
    for (; len; len--, a++)
    {
        *(u8 *)a = val;
    }
}

static inline void memcpy(void *a, const void *b, u32 len)
{
    for (; len; len--, a++, b++)
    {
        *(char *)a = *(const char *)b;
    }
}

static inline int get_update_in_progress_flag() {
      outb(0x70, 0x0A);
      return (inb(0x71) & 0x80);
}

static inline int read_sec()
{
    while (get_update_in_progress_flag())
        ;
    outb(0x70, 0x00);
    return inb(0x71);
}

static inline void delay()
{
    int sec = read_sec();

    while(read_sec() == sec)
        ;
}

static inline RSDP *FindRSDP()
{
    for (char *p = (char *)0x0E0000ULL; (u32)p < 0x0FFFF0ULL; p++)
    {
        if (memcmp(p, "RSD PTR ", 8) == 0)
            return (RSDP *)p;
    }

    return NULL;
}

static inline void PatchAcpiTable(void)
{
    int i, j, EntryCount;
    char strBuff[20];
    u64 *EntryPtr;

    RSDP *Root = FindRSDP();

    if (Root)
    {
        printf("RSDP Found at 0x%x\n", Root);
    }
    else
        return;

    printf("RSDT address= [0x%08x]\n", Root->rsdt);

    DESC_HEADER *rsdt = (DESC_HEADER *)Root->rsdt;

    int entries = (rsdt->length - 32) / 4;
    printf("RSDT have %d entries\n", entries);

    for (int i = 0; i < entries; i++)
    {
        DESC_HEADER *header = (DESC_HEADER *)(((u32 *)(rsdt->data))[i]);
        memcpy(strBuff, header->sig, 4);
        strBuff[4] = 0;
        printf("[%d] %s found at: %08x, length: %d\n", i, strBuff, header, header->length);

        if (memcmp(header->sig, "FACP", 4) == 0)
        {
            DESC_HEADER *dsdt_hdr = *(DESC_HEADER **)(header->data + 40 - 32);

            memcpy(strBuff, dsdt_hdr->sig, 4);

            for (int i = 0; i < dsdt_hdr->length - 32 - 6; i++)
            {
                if (memcmp(dsdt_hdr->data + i,
                           "\x08"
                           "ECFL"
                           "\xFF",
                           6) == 0)
                {
                    printf("ECFL defination found at: %08x, orig value: %02x, ", dsdt_hdr->data + i, (dsdt_hdr->data + i)[5]);
                    (dsdt_hdr->data + i)[5] = 0x01;
                    dsdt_hdr->checksum -= 0x02;
                    printf("new value: %02x\n", (dsdt_hdr->data + i)[5]);

                    int a = 0;
                    for ( i = 0; i < dsdt_hdr->length; i++)
                        a += ((unsigned char *)dsdt_hdr)[i];
                    a = a & 0xFF;
                    
                    printf("Checksum: %02x\n", a);
                    return;
                }
            }

            printf("ECFL not found...\n");

            return;
        }
    }

    printf("FACP not found...\n");
}

static inline u16 PciConfigReadWord(u8 bus, u8 slot, u8 func, u8 offset) {
    u32 address;
    u32 lbus  = (u32)bus;
    u32 lslot = (u32)slot;
    u32 lfunc = (u32)func;
    u16 tmp = 0;

    // Create configuration address as per Figure 1
    address = (u32)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((u32)0x80000000));

    // Write out the address
    outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (u16)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

static inline u32 PciCheckVendorDevice(u8 bus, u8 slot) {
    u16 vendor, device;
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    if ((vendor = PciConfigReadWord(bus, slot, 0, 0)) != 0xFFFF) {
       device = PciConfigReadWord(bus, slot, 0, 2);
       return ((u32) vendor << 16) + device;
    }

    return 0;
}

static inline int CheckPciBus()
{
    for (u32 bus = 0; bus < 256; bus++)
        for (u32 device = 0; device < 32; device++)
        {
            if (PciCheckVendorDevice(bus, device) == 0x80863B09)
            {
                printf("[8086:3b09] HM55 Chipset LPC Interface found...\n");
                printf("Patching ACPI Table...\n");

                return 1;
            }
        }

    printf("[8086:3b09] HM55 Chipset LPC Interface not found...\n");
    printf("Skip patching ACPI Table.\n");

    return 0;
}

int main(void)
{
    ScreenInit();
    // ClearScreen();

    if (CheckPciBus())
        PatchAcpiTable();

    putsln("Starting windows...");
    for (int i = 0; i < 3; i++)
        putchar('.'), delay();

    int pos = GetCursorPos();
    return pos / 80 * 256 + pos % 80;
}
