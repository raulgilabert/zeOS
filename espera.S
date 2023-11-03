#include <asm.h>

ENTRY(espera_larga)
    pushl %ebp
    movl %esp, %ebp

    xorl %edx, %edx

for_espera_larga:
    cmpl $1000000, %edx
    je exit_espera_larga

    xorl %ecx, %ecx

for_2_espera_larga:
    cmpl $100, %ecx
    je exit_2_espera_larga

    addl $1, %ecx
    jmp for_2_espera_larga

exit_2_espera_larga:
    addl $1, %edx
    jmp for_espera_larga

exit_espera_larga:
    popl %ebp
    ret