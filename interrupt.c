/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <entry.h>
#include <sys_libc.h>
#include <circ_buff.h>

#include <zeos_interrupt.h>

extern struct circ_buff cb;

Gate idt[IDT_ENTRIES];
Register    idtR;

extern struct task_struct *idle_task;
extern struct list_head readyqueue;
extern unsigned long zeos_ticks;
char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void syscall_handler_sysenter();

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */

	setInterruptHandler(33, keyboard_handler, 0);
	setInterruptHandler(32, clock_handler, 0);
	setInterruptHandler(14, custom_page_fault_handler, 0);

	
  setTrapHandler(0x80, system_call_handler, 3);

  setMSR(0x174, __KERNEL_CS);
  setMSR(0x176, (unsigned long)syscall_handler_sysenter);

  set_idt_reg(&idtR);
}


void custom_page_fault_routine(unsigned int error, unsigned int eip)
{
  char eip_value[8];
  uitox(eip, eip_value);

  printk("\nPagefault generated at \%eip=0x");
  printk(eip_value);
  while (1);
}

void keyboard_routine()
{
	unsigned char key_read = inb(0x60);
	unsigned char is_make = key_read & 0x80;

	if (is_make == 0x80)
	{
		unsigned char key_value = char_map[key_read & 0x7F];

    cb_add(&cb, key_value);

		/*if (key_value == '\0')
		{
			printc_xy(0, 0, 'C');
		}
    else if (key_value == 'k')
    {
      struct list_head *first = list_first(&readyqueue);
      struct task_struct *new_task = list_head_to_task_struct(first);
      task_switch( (union task_union *) new_task);
    }
		else
		{
		}*/
	}
}

void clock_routine()
{

  ++zeos_ticks;
	zeos_show_clock();

  schedule();
}
