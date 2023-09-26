#include <asm.h>

ENTRY(addASM)
	pushl %ebp
	movl %esp, %ebp
	movl 0x8(%ebp), %ecx
	movl 0xc(%ebp), %eax
	addl %ecx, %eax
	popl %ebp
	ret
