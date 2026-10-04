bits 32

section .bss
align 16


section .text

global _start
extern kernel_main
extern boot_magic


_start:

    ; --------------------------------------------------------
    ; Prove boot.asm was reached
    ; --------------------------------------------------------

    mov word [0xB8000], 0x074B


    ; --------------------------------------------------------
    ; Normal disk boot is NOT Multiboot
    ; --------------------------------------------------------

    mov dword [boot_magic], 0


    ; --------------------------------------------------------
    ; Disable interrupts
    ; --------------------------------------------------------

    cli


    ; --------------------------------------------------------
    ; Use a fixed stack.
    ;
    ; This is safely below the kernel at 0x100000.
    ; --------------------------------------------------------

    mov esp, 0x90000
    and esp, -16

    mov ebp, 0


    ; --------------------------------------------------------
    ; Prove we reached the C call
    ; --------------------------------------------------------

    mov word [0xB8002], 0x0743


    ; --------------------------------------------------------
    ; Call kernel_main using its absolute linked address.
    ; --------------------------------------------------------

    mov eax, kernel_main
    call eax


.hang:
    cli
    hlt
    jmp .hang


section .note.GNU-stack noalloc noexec nowrite progbits

