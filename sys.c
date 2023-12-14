/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#include <circ_buff.h>

#include <sem.h>

#define LECTURA 0
#define ESCRIPTURA 1
extern unsigned long zeos_ticks;
extern unsigned long quantum_ticks;
extern struct list_head freequeue, readyqueue;

extern struct circ_buff cb;

extern struct list_head keyboardqueue;

extern Byte x, y;

extern Byte color;

unsigned int next_pid = 2;

sem_t semaforos[MAX_SEMAFOROS];



int ret_from_fork()
{
  ready_to_system();
  return 0;
}


int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID = -1;

  if(list_empty(&freequeue))
  {
    return -1;
  }

  
  // Pillamos el task_struct si quedan disponibles
  struct list_head *first = list_first(&freequeue);
  struct task_struct *new_struct = list_head_to_task_struct(first);

  list_del(first);

  union task_union *new_union = (union task_union *) new_struct;
  
  // Pillamos el padre
  struct task_struct  *current_struct = current();
  union task_union *current_union = (union task_union *)current_struct;

  // copia de los datos de la pila de sistema
  copy_data(current_union, new_union, KERNEL_STACK_SIZE*4);

  // asignar espacio de directorio
  allocate_DIR(new_struct);

  page_table_entry *new_page_table = get_PT(new_struct);

  int new_frame;

  for (int i = 0; i < NUM_PAG_DATA; ++i)
  {
    new_frame = alloc_frame();
    if (new_frame != -1)
    {
      set_ss_pag(new_page_table, PAG_LOG_INIT_DATA + i, new_frame);
    }
    else // dealocatar todo
    {
      // hacemos el bucle hacia atrás
      for (; i >= 0; --i)
      {
        free_frame(get_frame(new_page_table, PAG_LOG_INIT_DATA + i));
        del_ss_pag(new_page_table, PAG_LOG_INIT_DATA + i);
      }
      list_add_tail(first, &freequeue);
      return -ENOMEM;
    }
  }

  page_table_entry *current_page_table = get_PT(current_struct);

  for (int i = 0; i < NUM_PAG_KERNEL; ++i)
  {
    set_ss_pag(new_page_table, i, get_frame(current_page_table, i));
  }

  for (int i = 0; i < NUM_PAG_CODE; ++i)
  {
    set_ss_pag(new_page_table, PAG_LOG_INIT_CODE + i, 
               get_frame(current_page_table, PAG_LOG_INIT_CODE + i));
  }

  for (int i = 0; i < NUM_PAG_DATA; ++i)
  {
    set_ss_pag(current_page_table, TOTAL_PAGES - NUM_PAG_DATA + i, 
               get_frame(new_page_table, PAG_LOG_INIT_DATA + i));

    copy_data((void *)((PAG_LOG_INIT_DATA + i) << 12), 
              (void *)((TOTAL_PAGES - NUM_PAG_DATA + i) << 12), PAGE_SIZE);

    del_ss_pag(current_page_table, TOTAL_PAGES - NUM_PAG_DATA + i);
  }

  //Flush TLB
  set_cr3(get_DIR(current()));

  new_union->stack[KERNEL_STACK_SIZE - 0x13] = 0;
  new_union->stack[KERNEL_STACK_SIZE - 0x12] = (unsigned long) &ret_from_fork;
  new_struct->kernel_esp = (unsigned long)&new_union->stack[KERNEL_STACK_SIZE - 0x13];

 // crear el proceso hijo
  PID=next_pid++;
  new_struct->PID= PID;
  new_struct->state = ST_READY;
  new_struct->threads_qtt = 1;

  initialize_stats(new_struct);

  list_add_tail(&(new_struct->list), &readyqueue);
  
  return PID;
}

void sys_exit()
{  
  struct task_struct *proc = current();

  proc->threads_qtt--;

  // eliminar la memoria allocada para datos
  page_table_entry *exit_proc_page_table = get_PT(proc);

  if (proc->threads_qtt == 0)
  {
    for (int i = 0; i < NUM_PAG_DATA; ++i)
    {
      free_frame(get_frame(exit_proc_page_table, PAG_LOG_INIT_DATA + i));
      del_ss_pag(exit_proc_page_table, PAG_LOG_INIT_DATA + i);
    }
  }
  else {
    for (int i = 0; i < proc->stack_size; ++i)
    {
      free_frame(get_frame(exit_proc_page_table, proc->base_stack + i));
      del_ss_pag(exit_proc_page_table, proc->base_stack + i);
    }
  }

  // liberar estructuras de datos del proceso
  proc->dir_pages_baseAddr = NULL;
  proc->PID = -1;

  // uso del scheduler para cambiar de proceso
  update_process_state_rr(proc, &freequeue);
  sched_next_rr();
}

#define BUFFER_SIZE 256
char buff[BUFFER_SIZE];

int sys_write(int fd, char *buffer, int size)
{
  int fd_e = check_fd(fd, ESCRIPTURA);
  if (fd_e)
  {
    return fd_e;
  }
  if (buffer == NULL)
  {
    return -EFAULT;
  }
  if (size < 0)
  {
    return -EINVAL;
  }

  int written = 0;
  int size_written = 0;

  while (size > BUFFER_SIZE)
  {
    copy_from_user(buffer, buff, BUFFER_SIZE);
    size_written += sys_write_console(buff, BUFFER_SIZE);

    size -= size_written;
    written += size_written;
  }

  if (size)
  {
    copy_from_user(buffer, buff, size);
    written += sys_write_console(buff, size);
  }

  return written;
}
int sys_gettime(){

  return zeos_ticks;
}

int sys_get_stats(int pid, struct stats *st)
{
  if (pid < 0)
  {
    return -EINVAL;
  }

  for (int i = 0; i < NR_TASKS; ++i)
  {
    if (task[i].task.PID == pid) 
    {
      task[i].task.stats.remaining_ticks = quantum_ticks;
      copy_to_user(&task[i].task.stats, st, sizeof(struct stats));
      return 0;
    }
  }

  return -ESRCH;
}

int sys_waitKey(char *b, int timeout)
{
  // comprobar que hay elementos en el buffer circular de teclado
  if (!cb_empty(&cb))
  {
    *b = cb_next(&cb);
    return 0;
  }

  // no hay elementos en el buffer circular de teclado
  // -------------------------------------------------
  // si el timeout es 0, devolver error
  if (timeout == 0)
  {
    return -EAGAIN;
  }

  // si el timeout es -1, bloquear indefinidadmente hasta que haya un elemento en el buffer circular de teclado
  if (timeout < 0)
  {
    // bloquear el proceso
    update_process_state_rr(current(), &keyboardqueue);
    sched_next_rr();
    // cuando se desbloquee, devolver el elemento del buffer circular de teclado
    *b = cb_next(&cb);
    return 0;
  }

  // si el timeout es mayor que 0, bloquear hasta que haya un elemento en el buffer circular de teclado o hasta que se cumpla el timeout

  // bloquear el proceso
  current()->timeout = timeout;

  update_process_state_rr(current(), &keyboardqueue);
  sched_next_rr();

  // al desbloquear se comprueba si hay elementos en el buffer circular de teclado
  if (!cb_empty(&cb))
  {
    // si hay elementos, devolver el elemento del buffer circular de teclado
    *b = cb_next(&cb);
    return 0;
  }
  else
  {
    // si no hay elementos, devolver error
    return -EAGAIN;
  }
}


int sys_goto_xy(int goto_x, int goto_y)
{
  if (goto_x < 0 || goto_x >= NUM_COLUMNS || goto_y < 0 || goto_y >= NUM_ROWS)
  {
    return -EINVAL;
  }

  x = goto_x;
  y = goto_y;

  return 0;
}

int sys_change_color(int fg, int bg)
{
  if (fg < 0 || fg > 15 || bg < 0 || bg > 7)
  {
    return -EINVAL;
  }

  color = 0 | bg << 4 | fg;

  return 0;
}

int sys_clrscr(char *b)
{
  Word *screen = (Word *)0xb8000;

  if (b == NULL)
  {
    for (int i = 0; i < NUM_COLUMNS; ++i)
    {
      for (int j = 0; j < NUM_ROWS; ++j)
      {
        screen[(j * NUM_COLUMNS + i)] = 0;
      }
    }
  }
  else
  {
    for (int i = 0; i < NUM_COLUMNS; i++)
    {
      for (int j = 0; j < NUM_ROWS; ++j)
      {
        screen[j * NUM_COLUMNS + i] = (b[(j * NUM_COLUMNS + i)*2 + 1] << 8) | 
                                         b[(j * NUM_COLUMNS + i)*2];
      }
    }
  }

  return 0;
}

int sys_thread_create_with_stack(void (*function)(void *arg), int N, void* parameter)
{

  if (N <= 0) // Tiene que tener stack
  {
    return -EINVAL;
  }

  // comprobar que quedan tasks libres
  if (list_empty(&freequeue))
  {
    return -ENOMEM;
  }

  current()->threads_qtt++;

  // Pillamos el task_struct si quedan disponibles
  struct list_head *first = list_first(&freequeue);
  struct task_struct *new_struct = list_head_to_task_struct(first);

  list_del(first);

  union task_union *new_union = (union task_union *) new_struct;

  // copia de los datos de la pila de sistema
  copy_data((union task_union *)current(), new_union, KERNEL_STACK_SIZE*4);


  new_struct->PID = current()->PID; // usamos el mismo PID que el proceso padre

  // usamos el mismo directorio que el proceso padre
  new_struct->dir_pages_baseAddr = current()->dir_pages_baseAddr;

  // copia de las tablas de páginas
  page_table_entry *pag_table = get_PT(new_struct);
  page_table_entry *parent_pag_table = get_PT(current());

  // copia de todas las páginas de la tabla padre a la hija a la vez que se busca N páginas
  // consecutivas disponibles
  int num_consecutives = 0;
  int first_consecutive = -1;
  int new_frame;
  int found = 0;

  for (int i = 0; i < TOTAL_PAGES; ++i)
  {
    if (!found)
    {
      if (pag_table[i].bits.present == 0)
      {
        if (num_consecutives == 0)
        {
          first_consecutive = i;
        }
        if (++num_consecutives == N)
        {
          found = 1;
        }
      }
      else
      {
        num_consecutives = 0;
      }
    }
  }

  if (!found) // mo hay páginas disponibles para el stack
  {
    list_add_tail(first, &freequeue);
    return -ENOMEM;
  }

  // allocatar las N páginas consecutivas
  for (int i = 0; i < N; ++i)
  {
    new_frame = alloc_frame();
    if (new_frame != -1)
    {
      set_ss_pag(pag_table, first_consecutive + i, new_frame);
    }
    else // dealocatar todo si no queda memoria física
    {
      // hacemos el bucle hacia atrás
      for (; i >= 0; --i)
      {
        free_frame(get_frame(pag_table, first_consecutive + i));
        del_ss_pag(pag_table, first_consecutive + i);
      }
      list_add_tail(first, &freequeue);
      return -ENOMEM;
    }
  }


  // configuración de la pila de sistema
  new_struct->kernel_esp = (unsigned long)&new_union->stack[KERNEL_STACK_SIZE - 0x12];

  // reescribir contexto hardware
  /*
  |--------|
  | EIP    | <- sobreescribimos con la nueva función
  |--------|
  | CS     | 
  |--------|
  | FLAGS  |
  |--------|
  | ESP    | <- sobreescribvimos con la nueva pila
  |--------|
  | SS     |
  |--------| 
  */

  //new_union->stack[KERNEL_STACK_SIZE - 0x2] = (unsigned long) parameter;

/*



%ebp
@ret
param1
*/

/*



%ebp
param1
*/



  unsigned long *user_stack = (first_consecutive) << 12;
  user_stack[N*1024 - 1] = (unsigned long) parameter;
  user_stack[N*1024 - 2] = (unsigned long) function;

  new_struct->base_stack = user_stack;
  new_struct->stack_size = N;


  new_union->stack[KERNEL_STACK_SIZE - 0x2] = (unsigned long) (user_stack + N*1024 - 2);

  new_union->stack[KERNEL_STACK_SIZE - 0x5] = (unsigned long) function;

  update_process_state_rr(new_struct, &readyqueue);

  // añadir elementos a la pila de usuario


  printk("a");

  return 0;

}


sem_t* sys_sem_create(int initial_value) {
  // comprobamos que initial_value no es negativo
  if (initial_value < 0)
  {
    return NULL;
  }

  // comprobamos que quedan semáforos libres
  int i;
  for (i = 0; i < MAX_SEMAFOROS; ++i)
  {
    if (semaforos[i].owner == NULL)
    {
      break;
    }
  }

  if (i == MAX_SEMAFOROS)
  {
    return NULL;
  }

  // inicializamos el semáforo
  semaforos[i].value = initial_value;
  semaforos[i].owner = current();

  INIT_LIST_HEAD(&semaforos[i].blocked);

  return &semaforos[i];
}


int sys_sem_wait(sem_t* s)
{
  // comprobamos que el semáforo no es nulo
  if (s == NULL)
  {
    return -EINVAL;
  }

  // decrementamos el contador del semáforo
  s->value--;

  // si el contador es negativo, bloqueamos el proceso
  if (s->value < 0)
  {
    update_process_state_rr(current(), &s->blocked);
    sched_next_rr();
  }

  return 0;
}

int sys_sem_signal(sem_t* s)
{
  // comprobamos que el semáforo no es nulo
  if (s == NULL)
  {
    return -EINVAL;
  }

  // incrementamos el contador del semáforo
  s->value++;

  // si el contador es negativo, desbloqueamos el primer proceso bloqueado
  if (s->value <= 0)
  {
    struct list_head *first = list_first(&s->blocked);
    struct task_struct *new_task = list_head_to_task_struct(first);
    update_process_state_rr(new_task, &readyqueue);
  }

  return 0;
}

int sys_sem_destroy(sem_t* s)
{
  // comprobamos que el semáforo no es nulo
  if (s == NULL)
  {
    return -EINVAL;
  }

  // comprobamos que el semáforo pertenece al proceso actual
  if (s->owner != current())
  {
    return -EPERM;
  }

  // comprobamos que no hay procesos bloqueados
  if (!list_empty(&s->blocked))
  {
    return -EBUSY;
  }

  // liberamos el semáforo
  s->owner = NULL;

  return 0;
}


char* sys_mem_reg_get(int num_pages)
{
  // allocata num_pages páginas consecutivas en el espacio de usuario
  // devolver la dirección virtual de la primera página
  // si no hay suficiente memoria, devolver NULL

  // comprobar que num_pages es positivo
  if (num_pages <= 0)
  {
    return NULL;
  }

  // comprobar que quedan páginas libres
  int num_consecutives = 0;
  int first_consecutive = -1;
  int found = 0;

  page_table_entry *pag_table = get_PT(current());

  for (int i = 0; i < TOTAL_PAGES; ++i)
  {
    if (!found)
    {
      if (pag_table[i].bits.present == 0)
      {
        if (num_consecutives == 0)
        {
          first_consecutive = i;
        }
        if (++num_consecutives == num_pages)
        {
          found = 1;
        }
      }
      else
      {
        num_consecutives = 0;
      }
    }
  }

  if (!found) // mo hay páginas disponibles para el stack
  {
    return NULL;
  }

  // allocatar las N páginas consecutivas
  for (int i = 0; i <= num_pages; ++i)
  {
    int new_frame = alloc_frame();
    if (new_frame != -1)
    {
      set_ss_pag(pag_table, first_consecutive + i, new_frame);
    }
    else // dealocatar todo si no queda memoria física
    {
      // hacemos el bucle hacia atrás
      for (; i >= 0; --i)
      {
        free_frame(get_frame(pag_table, first_consecutive + i));
        del_ss_pag(pag_table, first_consecutive + i);
      }
      return NULL;
    }
  }

  ((int *)first_consecutive)[0] = num_pages;

  return (char *)((first_consecutive + 1) << 12);
}


int sys_mem_reg_del(char* m)
{
  // elimina la región previamente allocatada en m liberando todos los recursos
  // si m no es una dirección válida, devolver -1

  // comprobar que m es una dirección válida
  if (get_frame(get_PT(current()), (unsigned long)m >> 12) == -1)
  {
    return -EINVAL;
  }

  // liberar las páginas
  page_table_entry *pag_table = get_PT(current());

  int *full_pages = (int *)(((unsigned long)m >> 12) + 1);

  int pages_to_dealloc = full_pages[0];

  for (int i = 0; i < pages_to_dealloc; ++i)
  {
    free_frame(get_frame(pag_table, ((unsigned long)m >> 12) - 1 + i));
    del_ss_pag(pag_table, ((unsigned long)m >> 12) - 1 + i);
  }

  return 0;
}