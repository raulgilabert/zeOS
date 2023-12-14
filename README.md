# zeOS

## Cosas a implementar

### E1

- [x] Interrupción de teclado
- [x] Interrupción de clock
- [x] page fault
- [x] write
- [x] gettime 


### Proyecto

#### Milestones

- [x] teclado guarda en circular buffer
- [x] waitKey
- [X] gotoXY
- [X] changeColor & clrscr
- [X] create_thread & exit
- [X] sync
- [X] memRegGet & memRegDel
- [ ] jueginho
- [ ] allocator a user




- [x] wait_key


#### wait_key

- [x] Comprobar buffer circular
    - [x] si hay elementos disponibles se devuelve inmediatamente
    - [x] si no se bloquea


##### Buffer circular

```C
struct Buffercircu
{
    char buff[x];
    char keybord; //escritura ==> max = user-1 posiciones de lo contrario no sabríamos si está lleno o 
    char user; //lectura ==> max = keyboard
};
```

- [x] init
- [x] empty
- [x] add
- [x] next

#### block

- [x] Mueve proceso a la lista de bloqueo
- [x] Pasamos al siguiente proceso

##### Funcionamiento del desbloqueo

Al bloquear se calcula el tick en el que se debe desbloquear a partir de un 
timeout que se le pasa. Con esto la función de  la interrupción de reloj va 
comprobando el tick actual con el tick de desbloqueo de cada proceso.

#### unblock

- [x] mueve el proceso bloqueado a la lista de ready




#### Imprimir texto con colores

Bits del buffer VGA:

15: blink
14-12: background
11-8: foreground
7-0: letra



```C
//Variables globales
char foreground;
char background;

int changeColor(int fg, int bg)
{

    foreground = fg%16; 
    background = bg%8;

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
    Word fg =foreground >> 8; 
    Word bg = background >> 12;
    Word ch = (Word) (c & 0x00FF) | fg | bg;
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
			move_line();
    {
   }
  }
}

```