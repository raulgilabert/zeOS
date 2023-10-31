#include <asm.h>

ENTRY(write)
    pushl %ebp
    movl %esp, %ebp
    
    pushl %ebx
    pushl %edx
    pushl %ecx
    
    movl 0x08(%ebp), %edx
    movl 0x0c(%ebp), %ecx
    movl 0x10(%ebp), %ebx

    movl $4, %eax

    int $0x80

    popl %ecx
    popl %edx
    popl %ebx
    /* 
    *  Si %eax >= 0 -> todo correcto
    *  Si %eax < 0 -> ha habido un error
    */
    cmpl $0, %eax
    jg write_end

    negl %eax
    movl %eax, errno
    movl $-1, %eax

write_end:

    popl %ebp
    ret

ENTRY(gettime)

    push %ebp
	mov %esp,%ebp


	movl $10, %eax

    int $0x80

    popl %ebp
    ret
ENTRY(getpid)

    push %ebp
	mov %esp,%ebp


	movl $20, %eax

    int $0x80

    popl %ebp
    ret
ENTRY(fork)

    push %ebp
	mov %esp,%ebp


	movl $2, %eax

    int $0x80
    
    cmpl $0, %eax
    jge fork_end

    negl %eax
    movl %eax, errno
    movl $-1, %eax
fork_end:
    popl %ebp 
    ret
