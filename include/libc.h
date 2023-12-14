/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

int write(int fd, char *buffer, int size);
int gettime();

void itoa(int a, char *b);
void ctox(char a, char *b);

int strlen(char *a);

int getpid();

int fork();

void exit();

int get_stats(int pid, struct stats *st);

int wait_key(char *b, int timeout);

int goto_xy(int x, int y);

int change_color(int fg, int bg);

int clrscr(char *b);

/*
 * This creates a new thread with a dynamically allocated stack size of 4096*N bytes that will
 * execute function ‘function’ passing it the parameter ‘ parameter’ as in ‘function(parameter)’. This
 * thread and its stack must be freed after calling the system call
*/
int threadCreateWithStack( void (*function)(void* arg), int N, void* parameter);


#endif  /* __LIBC_H__ */
