; boot/boot.asm

section .multiboot
align 4

MBALIGN  equ 1 << 0
MEMINFO  equ 1 << 1
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

dd MAGIC
dd FLAGS
dd CHECKSUM


section .bss
align 16

stack_bottom:
    resb 16384        ; 16 KB stack
stack_top:


section .text
global _start
extern kernel_main

_start:
    cli

    ; Set up stack
    mov esp, stack_top

    ; Align stack for C ABI
    and esp, -16

    ; Clear base pointer
    mov ebp, 0

    ; Call C kernel
    call kernel_main


.hang:
    cli
    hlt
    jmp .hang


section .note.GNU-stack noalloc noexec nowrite progbits
