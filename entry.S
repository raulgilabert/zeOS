/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %edx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %ebx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %ebx; \
      pushl %ecx; \
      pushl %edx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define RESTORE_ALL \
			popl %edx; \
			popl %ecx; \
			popl %ebx; \
			popl %esi; \
			popl %edi; \
			popl %ebp; \
			popl %eax; \
			popl %ds; \
			popl %es; \
			popl %fs; \
			popl %gs 

#define EOI \
			movb $0x20, %al ; \
			outb %al, $0x20 ;


ENTRY(keyboard_handler)
			SAVE_ALL

			call user_to_system
			EOI	
			
			call keyboard_routine

			call system_to_user
			RESTORE_ALL
			iret

ENTRY(clock_handler)
			SAVE_ALL
			call user_to_system

			EOI

			call clock_routine

			call system_to_user
			RESTORE_ALL
			iret

ENTRY(system_call_handler)
			SAVE_ALL

			pushl %eax
			call user_to_system
			popl %eax
			cmpl $0, %eax /* posible entrada negativa */
			jl error

			cmpl $MAX_SYSCALL, %eax /* posible entrada superior a la última */
			jge error

			call *sys_call_table(, %eax, 0x04)
			jmp end

error:
			movl $-ENOSYS, %eax

end:
			movl %eax, 0x18(%esp)

			call system_to_user

			RESTORE_ALL


			iret 


ENTRY(custom_page_fault_handler)
			call custom_page_fault_routine

			iret

ENTRY(setMSR)
	push %ebp
	mov %esp, %ebp

	mov 0x8(%ebp), %ecx # Paso de primer parametro = numero MSR
	movl $0, %edx       	# Paso de la parte baja del segundo parametro 
	mov 0xc(%ebp), %eax 
	wrmsr

	pop %ebp
	ret

ENTRY(syscall_handler_sysenter)
      push $__USER_DS
      push %ebp     # direccion stack del usuario 
      pushfl
      push $__USER_CS
      push 4(%ebp)  # direccion de retorno
      SAVE_ALL
      cmpl $0, %eax
      jl sysenter_error
      cmpl $MAX_SYSCALL, %eax
      jg sysenter_error
      call *sys_call_table(, %eax, 0x04)
      jmp sysenter_end

sysenter_error:
      movl $-ENOSYS, %eax # Si no existe la funcion

sysenter_end:
      movl %eax, 0x18(%esp)
      RESTORE_ALL
      movl (%esp), %edx # direcion de retorno
      movl 12(%esp), %ecx # direccion de la stack del usuario
      sti  
      sysexit