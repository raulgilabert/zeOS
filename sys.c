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

extern zeos_ticks;

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
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}

#define BUFFER_SIZE 256

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

  char buff[BUFFER_SIZE];

  int written = 0;

  while (size > BUFFER_SIZE)
  {
    copy_from_user(buffer, buff, BUFFER_SIZE);

    int size_written = sys_write_console(buff, BUFFER_SIZE);

    size -= size_written;
    buffer += size_written;
    written += size_written;
  }

  copy_from_user(buffer, buff, size);
  written += sys_write_console(buff, size);

  return written;
}

int sys_gettime(){

  return zeos_ticks;

}