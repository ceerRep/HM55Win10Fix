#include "io.h"
#include "util.h"

#define WIDTH 80
#define HEIGHT 25

#define VGA_BASE ((unsigned char *)0xB8000)

static int now_cursor_pos = 0;
static unsigned char now_output_color = 0x07;

int GetCursorPos()
{
    return now_cursor_pos;
}

void ScreenInit()
{
    u16 pos = 0;
    outb(0x3D4, 0x0F);
    pos |= inb(0x3D5);
    outb(0x3D4, 0x0E);
    pos |= ((u16)inb(0x3D5)) << 8;
    now_cursor_pos = pos;
}

void ScrollScreen(int line)
{
    unsigned char *to = VGA_BASE;
    unsigned char *from = VGA_BASE + 2 * WIDTH * line;

    while (from != VGA_BASE + 2 * WIDTH * HEIGHT)
    {
        *to++ = *from++;
    }

    while (to != VGA_BASE + 2 * WIDTH * HEIGHT)
    {
        *to++ = ' ';
        *to++ = now_output_color;
    }

    now_cursor_pos -= line * WIDTH;

    if (now_cursor_pos < 0)
        now_cursor_pos = 0;

    UpdateCursor(now_cursor_pos);
}

static inline void UpdateCursor_and_ensure_in_screen()
{
    if (now_cursor_pos >= WIDTH * HEIGHT)
    {
        int line = (now_cursor_pos - WIDTH * HEIGHT) / WIDTH + 1;
        ScrollScreen(line);
    }
    else
        UpdateCursor(now_cursor_pos);
}

void ClearScreen()
{
    unsigned char *now = VGA_BASE;

    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        *now++ = ' ';
        *now++ = now_output_color;
    }

    UpdateCursor(0);
}

void UpdateCursor(u16 pos)
{
    now_cursor_pos = pos;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

void putchar(char ch)
{
    if (ch == '\n')
    {
        now_cursor_pos = (now_cursor_pos / WIDTH + 1) * WIDTH;
    }
    else
    {
        *(unsigned char *)(VGA_BASE + 2 * now_cursor_pos) = ch;
        *(unsigned char *)(VGA_BASE + 2 * now_cursor_pos + 1) = now_output_color;
        now_cursor_pos++;
    }
    UpdateCursor_and_ensure_in_screen();
}

void puts(const char *string)
{
    while (*string)
    {
        putchar(*string++);
    }
}

void putsln(const char *string)
{
    puts(string);
    putchar('\n');
}