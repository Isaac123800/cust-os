ISO_NAME = Custos.iso

CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -ffreestanding -Iinclude -I.
LDFLAGS = -m elf_i386 -T linker.ld

all:
	mkdir -p build

	$(AS) -f elf32 boot/boot.asm -o build/boot.o

	$(CC) $(CFLAGS) -c kernel/kernel.c -o build/kernel.o
	$(CC) $(CFLAGS) -c drivers/io.c -o build/io.o
	$(CC) $(CFLAGS) -c drivers/ata.c -o build/ata.o
	$(CC) $(CFLAGS) -c drivers/disk.c -o build/disk.o
	$(CC) $(CFLAGS) -c fs/filesystem.c -o build/filesystem.o

	$(LD) $(LDFLAGS) \
		build/boot.o \
		build/kernel.o \
		build/io.o \
		build/ata.o \
		build/disk.o \
		build/filesystem.o \
		-o build/kernel.bin

	mkdir -p iso/boot/grub

	cp build/kernel.bin iso/boot/kernel.bin
	cp grub/grub.cfg iso/boot/grub/grub.cfg

	grub-mkrescue -o $(ISO_NAME) iso

clean:
	rm -rf build
	rm -rf iso
	rm -f $(ISO_NAME)

.PHONY: all clean
