#include <libc.h>

int
add(int par1, int par2)
{
  return par1 + par2;
}

void imprimir_datos(int in_pid)
{
  write(1, "--------------------", 20);
  char buff[16];
  struct stats stats;
  for(int i = 0; i < 16; ++i) buff[i] = 0;
  write(1, "\nPID: ", 6);
  itoa(in_pid, buff);
  write(1, buff, sizeof(buff));

  get_stats(in_pid, &stats);
  write(1,"\n User_ticks: " , 15);
  itoa(stats.user_ticks, buff);  
  write(1, buff, strlen(buff));

  write(1,"\n system_ticks: " , 17);
  itoa(stats.system_ticks, buff);  
  write(1, buff, strlen(buff));

  write(1,"\n ready_ticks: " , 16);
  itoa(stats.ready_ticks, buff);  
  write(1, buff, strlen(buff));

  write(1,"\n blocked_ticks: " , 18);
  itoa(stats.blocked_ticks, buff);  
  write(1, buff, strlen(buff));
  
  write(1,"\n elapsed_ticks: " , 18);
  itoa(stats.elapsed_total_ticks, buff);  
  write(1, buff, strlen(buff));

  write(1,"\n total_trans: " , 15);
  itoa(stats.total_trans, buff);  
  write(1, buff, strlen(buff));

  write(1,"\n remaining_ticks: " , 20);
  itoa(stats.remaining_ticks, buff);  
  write(1, buff, strlen(buff));

  write(1,"\n zeos_ticks: " , 15);
  itoa(gettime(), buff);  
  write(1, buff, strlen(buff));

  write(1, "\n--------------------\n", 22);
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. This register is a
     privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3":: "r" (0) ); */

  char b;
  int timeout = 10;

  int err = waitKey(&b, timeout);

  if (err < 0)
  {
    write(1, "AAAA", 4);
  }
  else {
    write(1, &b, 1);
  }


  while(1) {
  }
}
