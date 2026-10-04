bits 32

section .bss
align 16

stack_bottom:
    resb 16384

stack_top:


section .text

global _start

extern kernel_main


_start:

    ; --------------------------------------------------------
    ; K = kernel entry reached
    ; --------------------------------------------------------

    mov word [0xB8000], 0x074B


    ; --------------------------------------------------------
    ; Disable interrupts
    ; --------------------------------------------------------

    cli


    ; --------------------------------------------------------
    ; Set up kernel stack
    ; --------------------------------------------------------

    mov esp, stack_top

    and esp, -16

    mov ebp, 0


    ; --------------------------------------------------------
    ; Call C kernel
    ; --------------------------------------------------------

    call kernel_main


.hang:
    cli
    hlt
    jmp .hang


section .note.GNU-stack noalloc noexec nowrite progbits

