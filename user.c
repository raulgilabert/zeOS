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

    

  while(1) {
    char buff;

    
    int num = wait_key(&buff, 100);

    if (num >= 0)
    {
/*      write(1, "Se ha pulsado la tecla: ", 24);
      write(1, &buff, 1);
      write(1, "\n", 1);
*/
      if (buff == 'a' || buff == 'A')
      {
        char new_screen[80*25*2];
        for (int i = 0; i < 80*25*2; i+=2)
        {
          new_screen[i] = 'A';
          new_screen[i+1] = 0x02;
        }

        clrscr(new_screen);
      }

      else if (buff == 'c' || buff == 'C')
      {
        clrscr(0);
      }

/*      char buff2[16];
      for(int i = 0; i < 16; ++i) buff2[i] = 0;
      itoa(getpid(), buff2);
      write(1, "PID: ", 5);
      write(1, buff2, strlen(buff2));
      write(1, "\n", 1);*/
    }
    else
    {
      //write(1, "No se ha pulsado ninguna tecla\n", 31);
    }
  }
}
