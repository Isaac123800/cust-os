# cust-os
CustOS- The operating system where you have full control. You can use this in a vm.

Custos commands are used through the command prompt. The credits command displays the creators of Custos, Isaac Polomski, Roshan Inbasekar and Kirthis Kirubaventhan. The cmdlist command shows every available command and whether it is enabled, disabled, or protected; it cannot be disabled. The echo <text> command prints any text you enter back onto the screen, for example echo Hello displays Hello. The clear command clears the screen. The disable -<command> command disables a command so it cannot be used until it is enabled again, while the enable -<command> command restores a disabled command. The cmdlist, enable, and disable commands are protected and cannot be disabled. If you try to use a command that has been disabled, Custos will display error: Command not Found or not Enabled.

mkdir -p build
# Bootloader
nasm -f elf32 boot/boot.asm -o build/boot.o
# Kernel
gcc -m32 -ffreestanding -Iinclude -I. -c kernel/kernel.c -o build/kernel.o
# Installer
gcc -m32 -ffreestanding -Iinclude -I. -c installer/installer.c -o build/installer.o
gcc -m32 -ffreestanding -Iinclude -I. -c installer/install.c -o build/install.o
gcc -m32 -ffreestanding -Iinclude -I. -c installer/grub_install.c -o build/grub_install.o
# Drivers
gcc -m32 -ffreestanding -Iinclude -I. -c drivers/io.c -o build/io.o
gcc -m32 -ffreestanding -Iinclude -I. -c drivers/ata.c -o build/ata.o
gcc -m32 -ffreestanding -Iinclude -I. -c drivers/disk.c -o build/disk.o
gcc -m32 -ffreestanding -Iinclude -I. -c drivers/cdrom.c -o build/cdrom.o
# Filesystem
gcc -m32 -ffreestanding -Iinclude -I. -c fs/filesystem.c -o build/filesystem.o
In file included from fs/filesystem.h:4,
                 from fs/filesystem.c:1:
fs/filesystem.c: In function ‘fs_list’:
fs/../include/types.h:21:14: warning: ‘return’ with a value, in function returning void
   21 | #define true 1
      |              ^
fs/filesystem.c:1043:12: note: in expansion of macro ‘true’
 1043 |     return true;
      |            ^~~~
fs/filesystem.c:1010:6: note: declared here
 1010 | void fs_list(void)
      |      ^~~~~~~
gcc -m32 -ffreestanding -Iinclude -I. -c fs/iso9660.c -o build/iso9660.o
# Link kernel
ld -m elf_i386 -T linker.ld \
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
ld: warning: build/kernel.bin has a LOAD segment with RWX permissions
ld: build/install.o: in function `install_system':
install.c:(.text+0x60): undefined reference to `iso_list_root'
make: *** [Makefile:30: all] Error 1
