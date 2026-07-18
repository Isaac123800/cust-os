ISO_NAME = Custos.iso

CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -ffreestanding -Iinclude -I.
LDFLAGS = -m elf_i386 -T linker.ld


all:
	mkdir -p build


	# Bootloader
	$(AS) -f elf32 boot/boot.asm -o build/boot.o


	# Kernel
	$(CC) $(CFLAGS) -c kernel/kernel.c -o build/kernel.o


	# Installer
	$(CC) $(CFLAGS) -c installer/installer.c -o build/installer.o
	$(CC) $(CFLAGS) -c installer/install.c -o build/install.o
	$(CC) $(CFLAGS) -c installer/grub_install.c -o build/grub_install.o


	# Drivers
	$(CC) $(CFLAGS) -c drivers/io.c -o build/io.o
	$(CC) $(CFLAGS) -c drivers/ata.c -o build/ata.o
	$(CC) $(CFLAGS) -c drivers/disk.c -o build/disk.o
	$(CC) $(CFLAGS) -c drivers/cdrom.c -o build/cdrom.o


	# Filesystem
	$(CC) $(CFLAGS) -c fs/filesystem.c -o build/filesystem.o
	$(CC) $(CFLAGS) -c fs/iso9660.c -o build/iso9660.o



	# Link kernel
	$(LD) $(LDFLAGS) \
		build/boot.o \
		build/kernel.o \
		build/installer.o \
		build/install.o \
		build/grub_install.o \
		build/io.o \
		build/ata.o \
		build/disk.o \
		build/cdrom.o \
		build/filesystem.o \
		build/iso9660.o \
		-o build/kernel.bin



	# Create ISO structure
	rm -rf iso

	mkdir -p iso/boot/grub



	# GRUB kernel
	cp build/kernel.bin iso/boot/kernel.bin


	# Installer source file
	cp build/kernel.bin iso/KERNEL.BIN



	# GRUB configuration
	cp grub/grub.cfg iso/boot/grub/grub.cfg



	# Build ISO
	grub-mkrescue -o $(ISO_NAME) iso



clean:
	rm -rf build
	rm -rf iso
	rm -f $(ISO_NAME)



.PHONY: all clean
