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
    jge write_end

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

    push $gettime_ret
	push %ebp
	mov %esp,%ebp
    sysenter

gettime_ret:
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

ENTRY(exit)

    push %ebp
	mov %esp,%ebp

	movl $1, %eax

    int $0x80

    popl %ebp
    ret

ENTRY(goto_xy)
    push %ebp
	mov %esp,%ebp

    pushl %edx
    pushl %ecx
    
    movl 0x08(%ebp), %edx
    movl 0x0c(%ebp), %ecx

	movl $33, %eax

    int $0x80

    popl %ecx
    popl %edx

    cmpl $0, %eax
    jge goto_xy_end

    negl %eax
    movl %eax, errno
    movl $-1, %eax

goto_xy_end:

    popl %ebp
    ret


ENTRY(change_color)
    push %ebp
	mov %esp,%ebp

    pushl %edx
    pushl %ecx
    
    movl 0x08(%ebp), %edx
    movl 0x0c(%ebp), %ecx

	movl $34, %eax

    int $0x80

    popl %ecx
    popl %edx

    cmpl $0, %eax
    jge change_color_end

    negl %eax
    movl %eax, errno
    movl $-1, %eax

change_color_end:

    popl %ebp
    ret



ENTRY(clrscr)
    push %ebp
	mov %esp,%ebp

    pushl %edx
    
    movl 0x08(%ebp), %edx

	movl $32, %eax

    int $0x80

    popl %edx

    cmpl $0, %eax
    jge clrscr_end

    negl %eax
    movl %eax, errno
    movl $-1, %eax

clrscr_end:

    popl %ebp
    ret



ENTRY(get_stats)
    pushl %ebp
    movl %esp, %ebp
    
    pushl %edx
    pushl %ecx
    
    movl 0x08(%ebp), %edx
    movl 0x0c(%ebp), %ecx

    movl $35, %eax

    int $0x80

    popl %ecx
    popl %edx
    /* 
    *  Si %eax >= 0 -> todo correcto
    *  Si %eax < 0 -> ha habido un error
    */
    cmpl $0, %eax
    jge get_stats_end

    negl %eax
    movl %eax, errno
    movl $-1, %eax

get_stats_end:

    popl %ebp
    ret

ENTRY(wait_key)
    pushl %ebp
    movl %esp, %ebp
    
    pushl %edx
    pushl %ecx
    
    movl 0x08(%ebp), %edx
    movl 0x0c(%ebp), %ecx

    movl $24, %eax

    int $0x80

    popl %ecx
    popl %edx
    /* 
    *  Si %eax >= 0 -> todo correcto
    *  Si %eax < 0 -> ha habido un error
    */
    cmpl $0, %eax
    jge wait_key_end

    negl %eax
    movl %eax, errno
    movl $-1, %eax

wait_key_end:
    
    popl %ebp
    ret

ENTRY(threadCreateWithStack)
    pushl %ebp
    movl %esp, %ebp
    
    pushl %ebx
    pushl %edx
    pushl %ecx
    
    movl 0x08(%ebp), %edx
    movl 0x0c(%ebp), %ecx
    movl 0x10(%ebp), %ebx

    movl $30, %eax

    int $0x80

    popl %ecx
    popl %edx
    popl %ebx
    /* 
    *  Si %eax >= 0 -> todo correcto
    *  Si %eax < 0 -> ha habido un error
    */
    cmpl $0, %eax
    jge thread_create_end

    negl %eax
    movl %eax, errno
    movl $-1, %eax

thread_create_end:

    popl %ebp
    ret

