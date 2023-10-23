#include <asm.h>
#include <segment.h>

ENTRY(task_switch)
    pushl %ebp
    movl %esp, %ebp

    pushl %esi
    pushl %edi
    pushl %ebx

    pushl 8(%ebp) // se envía el PCB

    call inner_task_switch

    addl $4, %esp

    popl %ebx
    popl %edi
    popl %esi

    movl %ebp, %esp
    popl %ebp
    ret