#ifndef _UTIL_H

#define _UTIL_H

#include "define.h"

void outb(u16 port, u8 val);
void outw(u16 port, u16 val);
void outl(u16 port, u32 val);

u8 inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);

size_t strlen(const char* str);
char* itoa(int value, char* str, int base);

#endif
