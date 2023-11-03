/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

struct task_struct *idle_task;
struct list_head blocked;
struct list_head freequeue, readyqueue;
int quantum_ticks;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
		printc('A');
		
	}
}

void init_idle (void)
{
	struct list_head *first = list_first(&freequeue);

	list_del(first);

	struct task_struct *idle_struct = list_head_to_task_struct(first);
	
	// convertimos a union para tocar el stack

	union task_union *idle_union = (union task_union *) idle_struct;

	// ponemos valores
	idle_struct->PID = 0;

	set_quantum(idle_struct, QUANTUM);
	idle_struct->state = ST_READY;

	// damos el dir
	allocate_DIR(idle_struct);
	

	// Guardamos en la primera posición de la pila la dirección de cpu_idle
	idle_union->stack[KERNEL_STACK_SIZE - 1] = (unsigned long) &cpu_idle; 

	// Guardamos en la segunda posición de la pila el valor de %ebp (0)
	idle_union->stack[KERNEL_STACK_SIZE - 2] = (unsigned long) 0; 

	// Guardamos en kernel_esp la dirección del %ebp que guardamos en la pila
	idle_struct->kernel_esp = (unsigned long)&idle_union->stack[KERNEL_STACK_SIZE - 2];

	idle_task = idle_struct;
}

void init_task1(void)
{
	struct list_head *first = list_first(&freequeue);
	list_del(first);

	
	struct task_struct *init_struct = list_head_to_task_struct(first);
	union task_union *init_union = (union task_union *) init_struct;

	init_struct->PID = 1;
	set_quantum(init_struct, QUANTUM);
	init_struct->state = ST_RUN;

	quantum_ticks = QUANTUM;

	// damos el dir
	allocate_DIR(init_struct); 

	set_user_pages(init_struct);
	  tss.esp0 = (unsigned long)&(init_union->stack[KERNEL_STACK_SIZE]);
	//tss.esp0 = ((union task_union *)init_struct)->stack[KERNEL_STACK_SIZE];
	set_cr3(get_DIR(init_struct));

}


int get_quantum (struct task_struct *t)
{
	return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum)
{
	t->quantum = new_quantum;
}


void init_sched()
{
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);

	for (int i = 0; i < NR_TASKS; ++i)
	{
		list_add(&task[i].task.list, &freequeue);
	}

}

void inner_task_switch(union task_union *new) 
{
	tss.esp0 = KERNEL_ESP(new);
	set_cr3(new->task.dir_pages_baseAddr);
	
	current()->kernel_esp = get_ebp();

	set_esp(new->task.kernel_esp);
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void sched_next_rr()
{
	struct task_struct *next;

	if (!list_empty(&readyqueue))
	{
		struct list_head *first = list_first(&readyqueue);
		list_del(first);
		next = list_head_to_task_struct(first);

	}
	else
	{
		next = idle_task;
	}
	
	quantum_ticks = get_quantum(next);
	task_switch(next);
	
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest)
{

	if(!(t->list.next == NULL && t->list.prev ==NULL)) list_del(&(t->list));
	if (dest != NULL)
	{	
		if (t->PID != 0)
		{
			list_add_tail(&(t->list), dest);
		}
	}

}

int needs_sched_rr()
{
	// todavía no se ha llegado al quantum
	if (quantum_ticks > 0)
		return 0;

	// no quedan otros procesos en ready
	if (list_empty(&readyqueue))
	{
		quantum_ticks = get_quantum(current());
		return 0;
	}

	// en cualquiera de que no se cumplan esos dos casos se tiene que cambiar
	return 1;
}

void update_sched_data_rr()
{
	--quantum_ticks;
}

void schedule()
{
  update_sched_data_rr();
  if (needs_sched_rr())
  {
    update_process_state_rr(current(), &readyqueue);
    sched_next_rr();
  }
}
