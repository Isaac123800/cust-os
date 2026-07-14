ISO_NAME=Custos.iso

all:
	mkdir -p build

	nasm -f elf32 boot/boot.asm -o build/boot.o

	gcc -m32 -ffreestanding -IInclude -I. -c kernel/kernel.c -o build/kernel.o

	ld -m elf_i386 -T linker.ld build/boot.o build/kernel.o -o build/kernel.bin

	mkdir -p iso/boot/grub

	cp build/kernel.bin iso/boot/kernel.bin

	cp grub/grub.cfg iso/boot/grub/grub.cfg

	grub-mkrescue -o $(ISO_NAME) iso

clean:
	rm -rf build
	rm -rf iso
	rm -f $(ISO_NAME)
