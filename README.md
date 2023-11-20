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
- [ ] waitKey
- [ ] gotoXY
- [ ] changeColor & clrscr
- [ ] create_thread & exit
- [ ] sync
- [ ] memRegGet & memRegDel
- [ ] jueginho
- [ ] allocator a user




- [ ] wait_key


#### wait_key

- [ ] Comprobar buffer circular
    - [ ] si hay elementos disponibles se devuelve inmediatamente
    - [ ] si no se bloquea


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

- [ ] Mueve proceso a la lista de bloqueo
- [ ] Pasamos al siguiente proceso

##### Funcionamiento del desbloqueo

Al bloquear se calcula el tick en el que se debe desbloquear a partir de un 
timeout que se le pasa. Con esto la función de  la interrupción de reloj va 
comprobando el tick actual con el tick de desbloqueo de cada proceso.

#### unblock

- [ ] mueve el proceso bloqueado a la lista de ready