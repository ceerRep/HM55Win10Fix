SECTION .mbr

extern main
extern windows_mbr_pos
extern mbr_pos
extern gdt_start
extern text_pos

; [org 2000h]
[bits 16]

xor ax, ax
mov ss, ax
mov sp, mbr_pos

mov ds, ax
mov si, 0x7c00
mov es, ax
mov di, mbr_pos
mov cx, 512
cld
rep movsb

jmp dword 0x0:real_start

align 4

disk_read_packet:
db 16
db 0
sectors:
dw 2
dw windows_mbr_pos
dw 0
dd 1
dd 0

disk_id:
dw 2

real_start:
mov [disk_id], dx
mov ax, dx
shr ax, 4
and dx, 0xf
add ax, 0x0730
mov dx, 0x0730

push ds
push 0xb800
pop ds
mov [ds:(78 + 80 * 20) * 2], ax
mov [ds:(79 + 80 * 20) * 2], dx
pop ds

; 读磁盘
mov si, disk_read_packet
mov ah, 0x42
mov dx, [disk_id]
int 0x13

mov ax, [ds:text_pos]
mov [sectors], ax
mov ah, 0x42
mov dx, [disk_id]
mov si, disk_read_packet
int 0x13

; 空描述符
mov dword [gdt_start], 0x00
mov dword [gdt_start + 0x04], 0x00

; 代码段

; base 0-15 | size 0-15
mov dword [gdt_start + 0x08], 0x0000FFFF
;                             base31-24GDL_|size|PDPS|XCRA|base2316
mov dword [gdt_start + 0x0C], 00000000_1100_1111_1001_1010_00000000b

; 数据段
mov dword [gdt_start + 0x10], 0x0000FFFF
;                                                    |XEWA|
mov dword [gdt_start + 0x14], 00000000_1100_1111_1001_0010_00000000b

; 16bit 代码段

; base 0-15 | size 0-15
mov dword [gdt_start + 0x18], 0x0000FFFF
;                             base31-24GDL_|size|PDPS|XCRA|base2316
mov dword [gdt_start + 0x1C], 00000000_1000_1111_1001_1010_00000000b

; 16bit 数据段
mov dword [gdt_start + 0x20], 0x0000FFFF
;                                                    |XEWA|
mov dword [gdt_start + 0x24], 00000000_1000_1111_1001_0010_00000000b

mov word [gdt_size], 0x28

lgdt [cs: gdt_size]

; 关中断
cli

mov eax, cr0
or eax, 0x1
mov cr0, eax

jmp dword 0000000000001_000B:flush

[bits 32]

flush:
    mov ax, 0000000000010_000B
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x7C00

    mov word [0xB8002], 0x0762

    jmp start

start:

    call main

    mov esi, windows_mbr_pos
    mov edi, 0x7c00
    mov ecx, 440
    rep movsb

    jmp dword 0000000000011_000B:p16_code
[bits 16]
global p16_code
p16_code:
    mov bx, ax
    mov ax, 0000000000100_000B
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov eax, cr0
    and ax, ~1
    mov cr0, eax
    
    jmp dword 0x0:real_code

real_code:
    mov ax, 0x0
    mov ss, ax
    mov sp, 0x7c00
    mov ds, ax
    mov es, ax
    sti

    mov dx, bx
    mov bh, 0
    mov ah, 2
    int 0x10

    jmp dword 0x0:0x7c00
    gdt_size dw 0
    gdt_base dd gdt_start
