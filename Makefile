ISO_NAME = Custos.iso
DISK_NAME = work.img

CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

CFLAGS = -m32 -ffreestanding -fno-pie -fno-pic \
	-fno-stack-protector -fno-asynchronous-unwind-tables \
	-ffunction-sections -Iinclude -I.

LDFLAGS = -m elf_i386


all:
	mkdir -p build


	# ========================================================
	# SHARED CONSOLE
	# ========================================================

	$(CC) $(CFLAGS) \
		-c kernel/console.c \
		-o build/console.o


	# ========================================================
	# NORMAL CUSTOS KERNEL
	# ========================================================

	$(CC) $(CFLAGS) \
		-c kernel/kernel_main.c \
		-o build/kernel_main.o

	$(CC) $(CFLAGS) \
		-c drivers/io.c \
		-o build/io.o

	$(CC) $(CFLAGS) \
		-c drivers/ata.c \
		-o build/ata.o

	$(CC) $(CFLAGS) \
		-c drivers/disk.c \
		-o build/disk.o

	$(CC) $(CFLAGS) \
		-c fs/filesystem.c \
		-o build/filesystem.o


	# --------------------------------------------------------
	# Link installed kernel
	# --------------------------------------------------------

	$(LD) $(LDFLAGS) \
		-T linker.ld \
		build/kernel_main.o \
		build/console.o \
		build/io.o \
		build/ata.o \
		build/disk.o \
		build/filesystem.o \
		-o build/kernel.elf


	# --------------------------------------------------------
	# Raw installed kernel
	# --------------------------------------------------------

	$(OBJCOPY) -O binary \
		build/kernel.elf \
		build/kernel.bin


	# --------------------------------------------------------
	# Calculate installed kernel size
	# --------------------------------------------------------

	kernel_size=$$(stat -c %s build/kernel.bin); \
	kernel_sectors=$$(( (kernel_size + 511) / 512 )); \
	echo "Installed kernel: $$kernel_size bytes"; \
	echo "Installed kernel: $$kernel_sectors sectors"; \
	if [ $$kernel_sectors -gt 62 ]; then \
		echo "ERROR: kernel exceeds 62 BIOS sectors"; \
		exit 1; \
	fi


	# --------------------------------------------------------
	# Build BIOS bootloader using exact kernel size
	# --------------------------------------------------------

	kernel_size=$$(stat -c %s build/kernel.bin); \
	kernel_sectors=$$(( (kernel_size + 511) / 512 )); \
	$(AS) \
		-dKERNEL_SECTORS=$$kernel_sectors \
		-f bin \
		boot/bootloader.asm \
		-o build/bootloader.bin


	# ========================================================
	# INSTALLER KERNEL
	# ========================================================

	$(AS) -f elf32 \
		boot/multiboot.asm \
		-o build/multiboot.o


	$(CC) $(CFLAGS) \
		-c kernel/kernel_installer.c \
		-o build/kernel_installer.o


	# --------------------------------------------------------
	# Installer support
	# --------------------------------------------------------

	$(CC) $(CFLAGS) \
		-c installer/installer.c \
		-o build/installer.o

	$(CC) $(CFLAGS) \
		-c installer/install.c \
		-o build/install.o

	$(CC) $(CFLAGS) \
		-c installer/grub_install.c \
		-o build/grub_install.o


	# --------------------------------------------------------
	# Installer drivers
	# --------------------------------------------------------

	$(CC) $(CFLAGS) \
		-c drivers/io.c \
		-o build/io_installer.o

	$(CC) $(CFLAGS) \
		-c drivers/ata.c \
		-o build/ata_installer.o

	$(CC) $(CFLAGS) \
		-c drivers/disk.c \
		-o build/disk_installer.o

	$(CC) $(CFLAGS) \
		-c drivers/cdrom.c \
		-o build/cdrom.o


	# --------------------------------------------------------
	# Installer filesystem
	# --------------------------------------------------------

	$(CC) $(CFLAGS) \
		-c fs/filesystem.c \
		-o build/filesystem_installer.o

	$(CC) $(CFLAGS) \
		-c fs/iso9660.c \
		-o build/iso9660.o


	# --------------------------------------------------------
	# Link installer kernel
	# --------------------------------------------------------

	$(LD) $(LDFLAGS) \
		-T installer_linker.ld \
		build/multiboot.o \
		build/kernel_installer.o \
		build/console.o \
		build/io_installer.o \
		build/ata_installer.o \
		build/disk_installer.o \
		build/cdrom.o \
		build/filesystem_installer.o \
		build/iso9660.o \
		build/installer.o \
		build/install.o \
		build/grub_install.o \
		-o build/installer.elf


	# ========================================================
	# CREATE ISO
	# ========================================================

	rm -rf iso

	mkdir -p iso/boot/grub


	# Installer kernel
	cp build/installer.elf \
		iso/boot/kernel.bin


	# Normal kernel copied to disk by installer
	cp build/kernel.bin \
		iso/KERNEL.BIN


	# Custom BIOS bootloader
	cp build/bootloader.bin \
		iso/BOOTLOADER.BIN


	# GRUB configuration
	cp grub/grub.cfg \
		iso/boot/grub/grub.cfg


	# ========================================================
	# BUILD ISO
	# ========================================================

	grub-mkrescue \
		-o $(ISO_NAME) \
		iso


clean:
	rm -rf build
	rm -rf iso
	rm -f $(ISO_NAME)


.PHONY: all clean

