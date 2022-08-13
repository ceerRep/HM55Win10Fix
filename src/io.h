#ifndef _IO_H
#define _IO_H

#include "define.h"

int GetCursorPos();
void ScrollScreen(int line);
void ScreenInit();
void ClearScreen();
void UpdateCursor(u16 pos);

void putchar(char ch);
void puts(const char* string);
void putsln(const char* string);
void printint(int x, int base);

int printf(const char* format, ...);

#endif
