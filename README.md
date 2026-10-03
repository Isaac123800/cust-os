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

isaac@isaac-VMware-Virtual-Platform:~/cust-os$ objdump -h build/installer.elf

build/installer.elf:     file format elf32-i386

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         00003636  00100000  00100000  00001000  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .text.__x86.get_pc_thunk.ax 00000004  00103636  00103636  00004636  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .text.__x86.get_pc_thunk.cx 00000004  0010363a  0010363a  0000463a  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  3 .text.__x86.get_pc_thunk.bx 00000004  0010363e  0010363e  0000463e  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  4 .rodata       00000951  00103644  00103644  00004644  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  5 .eh_frame     00000c10  00103f98  00103f98  00004f98  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  6 .data         00000130  00104bc0  00104bc0  00005bc0  2**5
                  CONTENTS, ALLOC, LOAD, DATA
  7 .got.plt      0000000c  00104cf0  00104cf0  00005cf0  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  8 .bss          00010d60  00104d00  00104d00  00005cfc  2**5
                  ALLOC
  9 .comment      0000002d  00000000  00000000  00005cfc  2**0
                  CONTENTS, READONLY
isaac@isaac-VMware-Virtual-Platform:~/cust-os$ objdump -s -j .text build/installer.elf ~ head -20
objdump: invalid option -- '2'
Usage: objdump <option(s)> <file(s)>
 Display information from object <file(s)>.
 At least one of the following switches must be given:
  -a, --archive-headers    Display archive header information
  -f, --file-headers       Display the contents of the overall file header
  -p, --private-headers    Display object format specific file header contents
  -P, --private=OPT,OPT... Display object format specific contents
  -h, --[section-]headers  Display the contents of the section headers
  -x, --all-headers        Display the contents of all headers
  -d, --disassemble        Display assembler contents of executable sections
  -D, --disassemble-all    Display assembler contents of all sections
      --disassemble=<sym>  Display assembler contents from <sym>
  -S, --source             Intermix source code with disassembly
      --source-comment[=<txt>] Prefix lines of source code with <txt>
  -s, --full-contents      Display the full contents of all sections requested
  -Z, --decompress         Decompress section(s) before displaying their contents
  -g, --debugging          Display debug information in object file
  -e, --debugging-tags     Display debug information using ctags style
  -G, --stabs              Display (in raw form) any STABS info in the file
  -W, --dwarf[a/=abbrev, A/=addr, r/=aranges, c/=cu_index, L/=decodedline,
              f/=frames, F/=frames-interp, g/=gdb_index, i/=info, o/=loc,
              m/=macro, p/=pubnames, t/=pubtypes, R/=Ranges, l/=rawline,
              s/=str, O/=str-offsets, u/=trace_abbrev, T/=trace_aranges,
              U/=trace_info]
                           Display the contents of DWARF debug sections
  -Wk,--dwarf=links        Display the contents of sections that link to
                            separate debuginfo files
  -WK,--dwarf=follow-links
                           Follow links to separate debug info files (default)
  -WN,--dwarf=no-follow-links
                           Do not follow links to separate debug info files
  -L, --process-links      Display the contents of non-debug sections in
                            separate debuginfo files.  (Implies -WK)
      --ctf[=SECTION]      Display CTF info from SECTION, (default `.ctf')
      --sframe[=SECTION]   Display SFrame info from SECTION, (default '.sframe')
  -t, --syms               Display the contents of the symbol table(s)
  -T, --dynamic-syms       Display the contents of the dynamic symbol table
  -r, --reloc              Display the relocation entries in the file
  -R, --dynamic-reloc      Display the dynamic relocation entries in the file
  @<file>                  Read options from <file>
  -v, --version            Display this program's version number
  -i, --info               List object formats and architectures supported
  -H, --help               Display this information

 The following switches are optional:
  -b, --target=BFDNAME           Specify the target object format as BFDNAME
  -m, --architecture=MACHINE     Specify the target architecture as MACHINE
  -j, --section=NAME             Only display information for section NAME
  -M, --disassembler-options=OPT Pass text OPT on to the disassembler
  -EB --endian=big               Assume big endian format when disassembling
  -EL --endian=little            Assume little endian format when disassembling
      --file-start-context       Include context from start of file (with -S)
  -I, --include=DIR              Add DIR to search list for source files
  -l, --line-numbers             Include line numbers and filenames in output
  -F, --file-offsets             Include file offsets when displaying information
  -C, --demangle[=STYLE]         Decode mangled/processed symbol names
                                   STYLE can be "none", "auto", "gnu-v3",
                                   "java", "gnat", "dlang", "rust"
      --recurse-limit            Enable a limit on recursion whilst demangling
                                  (default)
      --no-recurse-limit         Disable a limit on recursion whilst demangling
  -w, --wide                     Format output for more than 80 columns
  -U[d|l|i|x|e|h]                Controls the display of UTF-8 unicode characters
  --unicode=[default|locale|invalid|hex|escape|highlight]
  -z, --disassemble-zeroes       Do not skip blocks of zeroes when disassembling
      --start-address=ADDR       Only process data whose address is >= ADDR
      --stop-address=ADDR        Only process data whose address is < ADDR
      --no-addresses             Do not print address alongside disassembly
      --prefix-addresses         Print complete address alongside disassembly
      --[no-]show-raw-insn       Display hex alongside symbolic disassembly
      --insn-width=WIDTH         Display WIDTH bytes on a single line for -d
      --adjust-vma=OFFSET        Add OFFSET to all displayed section addresses
      --show-all-symbols         When disassembling, display all symbols at a given address
      --special-syms             Include special symbols in symbol dumps
      --inlines                  Print all inlines for source line (with -l)
      --prefix=PREFIX            Add PREFIX to absolute paths for -S
      --prefix-strip=LEVEL       Strip initial directory names for -S
      --dwarf-depth=N            Do not display DIEs at depth N or greater
      --dwarf-start=N            Display DIEs starting at offset N
      --dwarf-check              Make additional dwarf consistency checks.
      --ctf-parent=NAME          Use CTF archive member NAME as the CTF parent
      --visualize-jumps          Visualize jumps by drawing ASCII art lines
      --visualize-jumps=color    Use colors in the ASCII art
      --visualize-jumps=extended-color
                                 Use extended 8-bit color codes
      --visualize-jumps=off      Disable jump visualization
      --disassembler-color=off       Disable disassembler color output. (default)
      --disassembler-color=terminal  Enable disassembler color output if displaying on a terminal.
      --disassembler-color=on        Enable disassembler color output.
      --disassembler-color=extended  Use 8-bit colors in disassembler output.

objdump: supported targets: elf64-x86-64 elf32-i386 elf32-iamcu elf32-x86-64 pei-i386 pe-x86-64 pei-x86-64 elf64-little elf64-big elf32-little elf32-big pe-bigobj-x86-64 pe-i386 pdb srec symbolsrec verilog tekhex binary ihex plugin
objdump: supported architectures: i386 i386:x86-64 i386:x64-32 i8086 i386:intel i386:x86-64:intel i386:x64-32:intel iamcu iamcu:intel

The following i386/x86-64 specific disassembler options are supported for use
with the -M switch (multiple options should be separated by commas):
  x86-64      Disassemble in 64bit mode
  i386        Disassemble in 32bit mode
  i8086       Disassemble in 16bit mode
  att         Display instruction in AT&T syntax
  intel       Display instruction in Intel syntax
  att-mnemonic  (AT&T syntax only)
              Display instruction with AT&T mnemonic
  intel-mnemonic  (AT&T syntax only)
              Display instruction with Intel mnemonic
  addr64      Assume 64bit address size
  addr32      Assume 32bit address size
  addr16      Assume 16bit address size
  data32      Assume 32bit data size
  data16      Assume 16bit data size
  suffix      Always display instruction suffix in AT&T syntax
  amd64       Display instruction in AMD64 ISA
  intel64     Display instruction in Intel64 ISA

Options supported for -P/--private switch:
For PE files:
  header      Display the file header
  sections    Display the section headers
isaac@isaac-VMware-Virtual-Platform:~/cust-os$ 

