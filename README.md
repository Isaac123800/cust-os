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

build/installer.elf:     file format elf32-i386

objdump: section '.multiboot' mentioned in a -j option, but not found in any input file

isaac@isaac-VMware-Virtual-Platform:~/cust-os$ objdump -s -j .text build/installer.elf | head -20

build/installer.elf:     file format elf32-i386

Contents of section .text:
 100000 02b0ad1b 00000000 fe4f52e4 66906690  .........OR.f.f.
 100010 fabc0000 090083e4 f0bd0000 0000e83f  ...............?
 100020 0d0000fa f4ebfc55 89e55653 83ec10e8  .......U..VS....
 100030 02360000 05bc4c00 00c745f4 01000000  .6....L...E.....
 100040 e9ae0000 00c745f0 00000000 e9940000  ......E.........
 100050 008b98d0 feffff8b 4df489ca c1e20201  ........M.......
 100060 cac1e204 89d18b55 f001ca01 d201d38b  .......U........
 100070 b0d0feff ff8b55f4 8d4aff89 cac1e202  ......U..J......
 100080 01cac1e2 0489d18b 55f001ca 01d28d0c  ........U.......
 100090 160fb613 88118b98 d0feffff 8b4df489  .............M..
 1000a0 cac1e202 01cac1e2 0489d18b 55f001ca  ............U...
 1000b0 01d283c2 0101d38b b0d0feff ff8b55f4  ..............U.
 1000c0 8d4aff89 cac1e202 01cac1e2 0489d18b  .J..............
 1000d0 55f001ca 01d283c2 018d0c16 0fb61388  U...............
 1000e0 118345f0 01837df0 4f0f8e62 ffffff83  ..E...}.O..b....
 1000f0 45f40183 7df4180f 8e48ffff ffc745ec  E...}....H....E.


