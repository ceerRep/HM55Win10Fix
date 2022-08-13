SECTIONS
{
    .defs :
    {
        INCLUDE ld_script_env;
    }
    . = 0x1000;
    gdt_start = ABSOLUTE(.);
    . = 0x2000;
    mbr_pos = ABSOLUTE(.);
    .mbr :
    {
        *(.mbr);
    }
    . = 0x2800;
    printf_buffer = ABSOLUTE(.);
    . = 0x3000;
    windows_mbr_pos = ABSOLUTE(.);
    . = . + 512;
    text_pos = ABSOLUTE(.);
    .text :
    {
        SHORT((SIZEOF(.text)+SIZEOF(.data)+511)/512+1);
        *(*startup);
        *(.text*);
    }
    .data :
    {
        *(.data*);
        *(.bss*);
        *(.rodata*);
    }
    .trash :
    {
        *(*);
    }
}
