# cust-os
CustOS- The operating system where you have full control. You can use this in a vm.

Custos commands are used through the command prompt. The credits command displays the creators of Custos, Isaac Polomski, Roshan Inbasekar and Kirthis Kirubaventhan. The cmdlist command shows every available command and whether it is enabled, disabled, or protected; it cannot be disabled. The echo <text> command prints any text you enter back onto the screen, for example echo Hello displays Hello. The clear command clears the screen. The disable -<command> command disables a command so it cannot be used until it is enabled again, while the enable -<command> command restores a disabled command. The cmdlist, enable, and disable commands are protected and cannot be disabled. If you try to use a command that has been disabled, Custos will display error: Command not Found or not Enabled.

drivers/cdrom.c: In function ‘cdrom_read_sector’:
drivers/cdrom.c:568:13: error: redefinition of ‘reason’
  568 |     uint8_t reason =
      |             ^~~~~~
drivers/cdrom.c:454:13: note: previous definition of ‘reason’ with type ‘uint8_t’ {aka ‘unsigned char’}
  454 |     uint8_t reason =
      |             ^~~~~~
make: *** [Makefile:25: all] Error 1

ld: warning: build/kernel.bin has a LOAD segment with RWX permissions
ld: build/iso9660.o: in function `iso_init':
iso9660.c:(.text+0x177): undefined reference to `cdrom_read_sector'
ld: build/iso9660.o: in function `iso_list_root':
iso9660.c:(.text+0x2d7): undefined reference to `cdrom_read_sector'
ld: build/iso9660.o: in function `iso_read_file':
iso9660.c:(.text+0x4a3): undefined reference to `cdrom_read_sector'
ld: iso9660.c:(.text+0x63b): undefined reference to `cdrom_read_sector'
make: *** [Makefile:30: all] Error 1

Installing for i386-pc platform.
grub-install: warning: disk does not exist, so falling back to partition device /dev/sda2.
grub-install: warning: disk does not exist, so falling back to partition device /dev/sda2.
grub-install: warning: disk does not exist, so falling back to partition device /dev/sda2.
grub-install: error: disk `hostdisk//dev/sda2' not found.

-rw-r--r-- 1 isaac isaac 512M Oct  2 18:04 work.img

Disk work.img: 512 MiB, 536870912 bytes, 1048576 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes


