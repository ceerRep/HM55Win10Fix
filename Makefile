CC=gcc
LD=ld
OBJCOPY=objcopy
GDB=gdb

CFLAGS=-std=gnu11 -Os -nostdlib -masm=intel -m32 -march=i386 -mno-red-zone -fno-stack-protector -ffreestanding -fno-pie
LDFLAGS=-Os -Wl,--nmagic,--script=bin.ld

SRC=${wildcard src/*.c}
OBJ=${patsubst src/%.c, obj/%.o, $(SRC)}

CODE_SECTOR?=2

all: hdd.img mbr.bin

hdd.img: loader.bin code.bin
	# dd if=/dev/zero of=hdd.img bs=1k count=1440
	dd if=loader.bin of=hdd.img bs=1 count=440 conv=notrunc
	dd if=nt6mbr.bin of=hdd.img seek=1 conv=notrunc
	dd if=code.bin of=hdd.img bs=512 seek=$(CODE_SECTOR) conv=notrunc

mbr.bin: loader.bin nt6mbr.bin code.bin
	dd if=loader.bin of=mbr.bin bs=1 count=440 conv=sync
	dd if=/dev/zero of=mbr.bin seek=440 count=82 conv=sync
	dd if=nt6mbr.bin of=mbr.bin seek=1 bs=512 conv=sync
	dd if=code.bin of=mbr.bin bs=512 seek=$(CODE_SECTOR) conv=sync

loader.bin: code.elf
	$(OBJCOPY) -j .mbr -O binary code.elf loader.bin

obj/mbr.o: mbr.s
	nasm mbr.s -o obj/mbr.o -f elf32

code.elf:  $(OBJ) obj/mbr.o bin.ld ld_script_env
	$(LD) -m elf_i386 --script=bin.ld $(OBJ) obj/mbr.o -o code.elf

code.bin: $(OBJ) code.elf
	$(OBJCOPY) -j .text -j .data -O binary code.elf code.bin

ld_script_env:
	echo "_code_sector = $(CODE_SECTOR);" > ld_script_env

$(OBJ) : obj/%.o : src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

qemu : hdd.img
	qemu-system-i386 hdd.img 

qemu-debug : hdd.img
	qemu-system-i386 -s -S hdd.img 

debug :
	$(GDB) code.elf -ex 'set disassembly-flavor intel' -ex 'target remote localhost:1234' -ex 'set tdesc filename target.xml' -ex 'set architecture i8086' -ex 'break *0x7c00' -ex 'continue' -ex 'display /3i $$pc'
clean :
	$(RM) obj/*.o mbr.bin code.elf header.bin code.bin loader.bin ld_script_env

.PHONY : all clean debug qemu qemu-debug
