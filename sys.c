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

#define LECTURA 0
#define ESCRIPTURA 1
extern int zeos_ticks;
extern struct list_head freequeue, readyqueue;

unsigned int next_pid = 2;


int ret_from_fork()
{
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

  // copia de los datos de la pila
  copy_data(current_union, new_union, KERNEL_STACK_SIZE*4);

  // asignar espacio de directorio
  allocate_DIR(new_struct);

  page_table_entry *new_page_table = get_PT(new_struct);

  int i = 0;
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
    set_ss_pag(new_page_table, PAG_LOG_INIT_CODE + i, get_frame(current_page_table, PAG_LOG_INIT_CODE + i));
  }

  for (int i = 0; i < NUM_PAG_DATA; ++i)
  {
    set_ss_pag(current_page_table, TOTAL_PAGES - NUM_PAG_DATA + i, get_frame(new_page_table, PAG_LOG_INIT_DATA + i));

    copy_data((void *)((PAG_LOG_INIT_DATA + i) << 12), (void *)((TOTAL_PAGES - NUM_PAG_DATA + i) << 12), PAGE_SIZE);

    del_ss_pag(current_page_table, TOTAL_PAGES - NUM_PAG_DATA + i);
  }

  //Flush TLB
  set_cr3(get_DIR(current()));

  new_union->stack[KERNEL_STACK_SIZE - 0x13] = 0;
  new_union->stack[KERNEL_STACK_SIZE - 0x12] = (unsigned long) &ret_from_fork;
  new_struct->kernel_esp = (unsigned long)&new_union->stack[KERNEL_STACK_SIZE - 0x13];

 // creates the child process
  PID=next_pid++;
  new_struct->PID= PID;
  new_struct->state = ST_READY;
  list_add_tail(&(new_struct->list), &readyqueue);
  
  return PID;
}

void sys_exit()
{  
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
