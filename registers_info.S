#include <asm.h>
ENTRY(get_ebp)
    movl %ebp, %eax
    ret
 