ISO_NAME = Custos.iso
DISK_NAME = work.img

CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

CFLAGS = -m32 -ffreestanding -Iinclude -I.
LDFLAGS = -m elf_i386 -T linker.ld

all:
	mkdir -p build

	if [ ! -f $(DISK_NAME) ]; then \
		qemu-img create -f raw $(DISK_NAME) 512M; \
	fi

	# Custom BIOS bootloader
	$(AS) -f bin boot/bootloader.asm -o build/bootloader.bin

	# Installed CustOS kernel entry - NO Multiboot
	$(AS) -f elf32 boot/boot.asm -o build/boot.o

	# Installer kernel entry - WITH Multiboot
	$(AS) -f elf32 boot/multiboot.asm -o build/multiboot.o

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

	# ------------------------------------------------
	# Build installed CustOS kernel
	# NO Multiboot header
	# ------------------------------------------------

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
		-o build/kernel.elf

	$(OBJCOPY) -O binary \
		build/kernel.elf \
		build/kernel.bin

	# ------------------------------------------------
	# Build installer kernel
	# WITH Multiboot header
	# ------------------------------------------------

	$(LD) $(LDFLAGS) \
		build/multiboot.o \
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
		-o build/installer.elf

	# ------------------------------------------------
	# Create ISO
	# ------------------------------------------------

	rm -rf iso
	mkdir -p iso/boot/grub

	# GRUB installer kernel
	cp build/installer.elf iso/boot/kernel.bin

	# Kernel that the installer will copy to the disk
	cp build/kernel.bin iso/KERNEL.BIN

	# Custom BIOS bootloader
	cp build/bootloader.bin iso/BOOTLOADER.BIN

	# GRUB configuration
	cp grub/grub.cfg iso/boot/grub/grub.cfg

	# Build installer ISO
	grub-mkrescue -o $(ISO_NAME) -- -iso-level 3 iso


clean:
	rm -rf build
	rm -rf iso
	rm -f $(ISO_NAME)


.PHONY: all clean
