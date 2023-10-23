/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <list.h>
#include <registers_info.h>

/* declaración del conjunto de estructuras de datos para las tareas,
 * guardado en la sección .data.task de memoria
 *
 * El proceso en la posición 0 del vector es el proceso idle y no
 * puede ser destruido
 */
union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, lista);
}
#endif

extern struct list_head blocked;
struct list_head freequeue, readyqueue;

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
	;
	}
}

void init_idle (void)
{
	/* Se recoje el primer elemento de la cola freequeue */
	struct list_head *first = list_first(&freequeue);
	/* Se elimina de la cola freequeue */
	list_del(first);

	/* Se convierte a task_struct */
	struct task_struct *idle_task = list_head_to_task_struct(first);

	/* Se asigna el PID 0 */
	idle_task->PID = 0;

	/* Se inicializa dir_pages_baseAaddr con un nuevo directorio para
	almacenar el proceso con allocate_DIR */
	allocate_DIR(idle_task);

	/* Se inicializa el contexto de ejecución para ejecutar la función
	cpu_idle */
	union task_union *idle_union = (union task_union*)idle_task;

	/* Dirección del código de la función cpu_idle */
	idle_union->stack[KERNEL_STACK_SIZE-1] = (unsigned long)&cpu_idle; 
	/* Valor de %ebp inicializado a 0 por ser la posición inicial de la
	pila del proceso */
	idle_union->stack[KERNEL_STACK_SIZE-2] = 0;
	/* Se guarda en el parámetro kernel_esp del PCB la posición de memoria
	del registro %ebp que se cargará siempre que se haga el cambio de
	contexto */
	idle_union->task.kernel_esp = (unsigned long)&(idle_union->stack[KERNEL_STACK_SIZE-2]);
}

void init_task1(void)
{
	/* Se recoje el primer elemento de la cola freequeue */
	struct list_head *first = list_first(&freequeue);
	/* Se elimina de la cola freequeue */
	list_del(first);

	/* Se convierte a task_struct */
	struct task_struct *init_task = list_head_to_task_struct(first);

	/* Se asigna el PID 1 */
	init_task->PID = 1;

	/* Se inicializa dir_pages_baseAaddr con un nuevo directorio para
	almacenar el proceso con allocate_DIR */
	allocate_DIR(init_task);

	/* se completa la inicialización del espacio de direcciones con la
	función set_user_pages */
	set_user_pages(init_task);

	/* Actalizar TSS para que apunte a la pila de sistema de la tarea y
	el número 0x175 de la MSR */
	tss.esp0 = KERNEL_ESP((union task_union *)init_task);
	writeMSR(0x175, (unsigned int) tss.esp0);

	/* Asignar la página como la actual en el sistema usando set_cr3 */
	set_cr3(init_task->dir_pages_baseAddr);
}


void init_sched()
{
	/* Inicializa la lista freequeue y readyqueue, además añadiendo
	 * todas las tareas a la lista freequeue. La lista readyqueue
	 * se inicializa vacía y se irá llenando con las tareas que
	 * estén listas para ejecutarse o continuar su ejecución.
	 */
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);
	for(int i = 0;i < NR_TASKS; ++i) list_add(&task[i].task.lista, &freequeue);
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

void inner_task_switch(union task_union* new)
{
	/* restaura la ejecución del proceso new en el mismo estado
	que estaba antes de invocar a la rutina. Se cambia a la nueva
	pila de kernel, deshacer el enlace dinámico en la pila y continuar
	la ejecución en la dirección del código de la cima de la pila */

	/* Se actualiza el puntero de la pila de sistema */
	tss.esp0 = new->task.kernel_esp;
	/* Se actualiza el valor 0x175 del MSR */
	writeMSR(0x175, (unsigned int) tss.esp0);
	/* Se actualiza el puntero de la tabla de páginas */
	set_cr3(new->task.dir_pages_baseAddr);

	/* se guarda el valor actual de %ebp en el PCB */
	current()->kernel_esp = get_ebp();
}


