#include <libc.h>

char buff[24];

int pid;

int
add(int par1, int par2)
{
  return par1 + par2;
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a
       privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  //write(1, "A", 1);
  int i = 0;

  int pid = fork();


  if (pid < 0)
    write(1, "AAAAA", 5);

    write(1, "mi PID es: ", 10);

    char buff[16];

    itoa(pid, buff);
    write(1, buff, strlen(buff));
    write(1, "\n", 1);

  while(1) {
  }
}
