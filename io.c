/*
 * io.c - 
 */

#include <io.h>

#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;

/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

void move_line()
{
	x = 0;
	if (y < 24)
	{
		y=y+1;
	}
	else
  {
		Word *screen = (Word *)0xb8000;
		for (int i = 0; i < 24; ++i)
		{
			for (int j = 0; j < 80; ++j)
			{
				screen[i*NUM_COLUMNS + j] = screen[(i + 1)*NUM_COLUMNS + j];
			}
		}
		for (int i = 0; i < 80; ++i)
		{
			screen[24*NUM_COLUMNS + i] = 0x0000;
		}
	}
}

void printc(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
		move_line();
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x0200;
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
			move_line();
    {
   }
  }
}

void printc_color(char c, char color)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
		move_line();
  }
  else
  {
		Word col = color << 2;
    Word ch = (Word) (c & 0x00FF) | col;
		Word *screen = (Word *)0xb8000;
		screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
			move_line();
    }
  }
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}
