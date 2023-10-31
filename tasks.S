#include <asm.h>

ENTRY(task_switch)
    pushl %ebp
    movl %esp, %ebp

    
    pushl %esi
    pushl %edi
    pushl %ebx
    
    pushl 8(%ebp)

    call inner_task_switch

    addl $4, %esp

    popl %ebx
    popl %edi
    popl %esi

    movl %ebp, %esp
    popl %ebp
    ret

ENTRY(get_ebp)
    movl %esp, %eax
    ret

ENTRY(set_esp)
    movl 4(%esp), %esp
    popl %ebp
    ret

ENTRY(cambio_punteros_stack)
    movl 4(%esp), %eax 
    movl %ebp, (%eax)
    movl 8(%esp), %esp
    popl %ebp
    ret