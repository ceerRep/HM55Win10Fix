#include "util.h"
#include "io.h"

void outb(u16 port, u8 val)
{
    asm volatile("out %1, %0"
                 :
                 : "a"(val), "Nd"(port));
}

void outw(u16 port, u16 val)
{
    asm volatile("out %1, %0"
                 :
                 : "a"(val), "Nd"(port));
}

void outl(u16 port, u32 val)
{
    asm volatile("out %1, %0"
                 :
                 : "a"(val), "Nd"(port));
}

u8 inb(u16 port)
{
    u8 ret;
    asm volatile("in %0, %1"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

u16 inw(u16 port)
{
    u16 ret;
    asm volatile("in %0, %1"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

u32 inl(u16 port)
{
    u32 ret;
    asm volatile("in %0, %1"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

size_t strlen(const char* str)
{
    const char* str1 = str;
    while (*str1)
        str1++;
    return str1 - str;
}
