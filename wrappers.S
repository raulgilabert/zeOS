#include <asm.h>

ENTRY(write)
    pushl %ebp
    movl %esp, %ebp

    movl 0x08(%ebp), %edx
    movl 0x0c(%ebp), %ecx
    movl 0x10(%ebp), %ebx

    movl $4, %eax

    int $0x80

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

    cmpl $0, %eax
    jg gettime_end

    negl %eax
    movl %eax, errno
    movl $-1, %eax

gettime_end:

    popl %ebp
    ret
    