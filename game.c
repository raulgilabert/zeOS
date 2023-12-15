#include <libc.h>
#include <sem.h>
#include <game.h>
#include <list.h>

struct keyboard_thread_data {
    sem_t *sem_keyboard;
    sem_t *sem_game;
    char l;

};

void keyboard_thread(struct keyboard_thread_data* d)
{

    char b = 0;
    while (1) {
        semWait(d->sem_keyboard);
        wait_key(&b, -1);
        d->l = b;
        semSignal(d->sem_game);
    }
}

struct zeldo {
    unsigned int x;
    unsigned int y;
    int hp;
};

struct enemigo {
    unsigned int x;
    unsigned int y;
    struct list_head list;
};

struct enemigos_data {
    int qtt_alive;
    int qtt;
    struct list_head enemigos;
};

struct enemigo *list_head_to_enemigo_struct(struct list_head *l)
{
    return list_entry(l, struct enemigo, list);
}

void render(char *tablero, int points, struct zeldo zeldo_data, struct enemigos_data *enemigos)
{
    char pantalla[80*25*2];

    for (int i = 0; i < 80*25; ++i) {
        pantalla[2*i] = tablero[i];

        switch (tablero[i]) {
            case '@':
                pantalla[2*i+1] = 0x26;
                break;
            case '#':
                pantalla[2*i+1] = 0x2a;
                break;
            default:
                pantalla[2*i+1] = 0x20;
        }
    }

    for (int i = 0; i < 80; ++i)
    {
        pantalla[2*i] = 0x0;
        pantalla[2*i+1] = 0x0;
    }

    clrscr(pantalla);

    goto_xy(zeldo_data.x, zeldo_data.y);
    change_color(0x00, 0x02);
    write(1, "Z", 1);

    change_color(0x02, 0x00);
    goto_xy(0, 0);
    write(1, "Puntos: ", 8);
    char b[10];
    for (int i = 0; i < 10; ++i) b[i] = 0;
    itoa(points, b);
    write(1, b, strlen(b));

    goto_xy(20, 0);
    write(1, "Vidas: ", 7);
    for (int i = 0; i < 10; ++i) b[i] = 0;
    itoa(zeldo_data.hp, b);
    write(1, b, strlen(b));

    struct list_head *e;

    if (!list_empty(&enemigos->enemigos)) {
        list_for_each(e, &enemigos->enemigos) {
            struct enemigo *en = list_head_to_enemigo_struct(e);
            goto_xy(en->x, en->y);
            change_color(0x04, 0x02);
            write(1, "E", 1);
        }
    }
}

void game() {
    // inicialización
    sem_t *sem_keyboard = semCreate(0);
    sem_t *sem_game = semCreate(0);

    struct keyboard_thread_data d;
    d.sem_keyboard = sem_keyboard;
    d.sem_game = sem_game;

    int res = threadCreateWithStack(keyboard_thread, 1, &d);

    write(1, "la leyenda de zeldo (nombre cambiado para respetar derechos de autor)\n\n", 71);

    write(1, "Historia:\n", 10);
    write(1, "Zeldo es un heroe que debe salvar a la princesa.\n", 49);
    write(1, "Pero este juego no tiene princesa por falta de recursos y tiempo, asi que Zeldo se va a dar una vuelta para subir de nivel y matar a los enemigos que se encuentre por el camino porque de esto va realmente el videojuego.\n", 220);

    write(1, "\n", 1);

    write(1, "Controles:\n", 11);
    write(1, "  - WASD para moverse\n", 22);
    write(1, "  - [Espacio] para atacar\n", 26);

    write(1, "\n", 1);

    write(1, "Objetivo:\n", 10);
    write(1, "  - Matar a todos los enemigos\n", 31);

    write(1, "\n", 1);

    write(1, "Los enemigos se mueven por la pantalla de forma aleatoria cada 5 movimientos de Zeldo.\n", 87);
    write(1, "Si Zeldo se coloca en la misma casilla que un enemigo, este le atacara y morira al instante.\n", 93);
    write(1, "Zeldo tiene 3 puntos de vida.\n", 30);
    write(1, "Si Zeldo se queda sin vida, el juego termina.\n", 46);
    write(1, "Zeldo puede atacar a los enemigos con su espada a una distancia de 1 casilla incluyendo diagonales.\n", 100);
    write(1, "Si Zeldo mata a todos los enemigos, el juego termina.\n", 55);

    write(1, "\n", 1);

    write(1, "\n", 1);
    do {
        write(1, "Pulsa [Enter] para continuar\n", 29);
        semSignal(sem_keyboard);
        semWait(sem_game);
    } while (d.l != '\n');

    char tablero[80*25];

    // Generación del tablero
    for (int i = 80; i < 80*25; ++i) {
        tablero[i] = ' ';
    }

    // generacion aleatoria de arbustos
    for (int i = 0; i < 100; ++i) {
        int x = rand()%80;
        int y = rand()%24 + 1;

        if (tablero[(y*80 + x)] == ' ') {
            tablero[(y*80 + x)] = '#';
        }
    }

    // generacion aleatoria de piedras
    for (int i = 0; i < 50; ++i) {
        int x = rand()%80;
        int y = rand()%24 + 1;

        if (tablero[(y*80 + x)] == ' ') {
            tablero[(y*80 + x)] = '@';
        }
    }


    write(1, "Escribe la cantidad de enemigos que quieres que haya en el juego (entre 1 y 100 no más que si no va muy lento): ", 114);

    char b[4];
    for (int i = 0; i < 3; ++i) b[i] = 0;

    int j = 0;

    do {
        semSignal(sem_keyboard);
        semWait(sem_game);
        write(1, &d.l, 1);
        if (j < 3)
        {
            b[j] = d.l;
            ++j;
        }
    } while (d.l != '\n');

    int qtt_enemigos = atoi(b);

    // Generación de enemigos
    struct enemigos_data enemigos;
    INIT_LIST_HEAD(&enemigos.enemigos);
    enemigos.qtt = qtt_enemigos;
    enemigos.qtt_alive = qtt_enemigos;

    for (int i = 0; i < enemigos.qtt; ++i) {
        struct enemigo *e = (struct enemigo*)memRegGet(1);

        do {
            e->x = rand()%80;
            e->y = rand()%24 + 1;
        } while (tablero[(e->y*80 + e->x)] != ' ');

        list_add_tail(&e->list, &enemigos.enemigos);
    }

    // Generación de Zeldo
    struct zeldo zeldo_data = {40, 11, 3};

    int points = 0;

    render(tablero, points, zeldo_data, &enemigos);

    int counter_movimiento = 0;

    while (1)
    {
        semSignal(sem_keyboard);
        semWait(sem_game);

        // movimiento de zeldo y ataque
        switch (d.l) {
            case 'w':
                // arriba
                if (zeldo_data.y > 1 &&
                    tablero[((zeldo_data.y-1)*80 + zeldo_data.x)] != '#' &&
                    tablero[((zeldo_data.y-1)*80 + zeldo_data.x)] != '@') {
                    zeldo_data.y--;
                }
                break;
            case 'a':
                // izquierda
                if (zeldo_data.x > 0 &&
                    tablero[(zeldo_data.y*80 + zeldo_data.x-1)] != '#' &&
                    tablero[(zeldo_data.y*80 + zeldo_data.x-1)] != '@') {
                    zeldo_data.x--;
                }
                break;
            case 's':
                // abajo
                if (zeldo_data.y < 24 &&
                    tablero[((zeldo_data.y+1)*80 + zeldo_data.x)] != '#' &&
                    tablero[((zeldo_data.y+1)*80 + zeldo_data.x)] != '@') {
                    zeldo_data.y++;
                }
                break;
            case 'd':
                // derecha
                if (zeldo_data.x < 79 &&
                    tablero[(zeldo_data.y*80 + zeldo_data.x+1)] != '#' &&
                    tablero[(zeldo_data.y*80 + zeldo_data.x+1)] != '@') {
                    zeldo_data.x++;
                }
                break;
            case ' ': {
                // ataque

                struct list_head *e = list_first(&enemigos.enemigos);
                
                for (; e != &enemigos.enemigos;) {
                    int deleted = 0;
                    struct enemigo *en = list_head_to_enemigo_struct(e);
                    // arriba
                    if (zeldo_data.y-1 == en->y &&
                        zeldo_data.x == en->x) {
                        deleted = 1;
                        e = e->next;
                        list_del(&en->list);
                        points++;
                        enemigos.qtt_alive--;
                    }
                    // izquierda
                    else if (zeldo_data.y == en->y &&
                        zeldo_data.x-1 == en->x) {
                        deleted = 1;
                        e = e->next;
                        list_del(&en->list);
                        points++;
                        enemigos.qtt_alive--;
                    }
                    // abajo
                    else if (zeldo_data.y+1 == en->y &&
                        zeldo_data.x == en->x) {
                        deleted = 1;
                        e = e->next;
                        list_del(&en->list);
                        points++;
                        enemigos.qtt_alive--;
                    }
                    // derecha
                    else if (zeldo_data.y == en->y &&
                        zeldo_data.x+1 == en->x) {
                        deleted = 1;
                        e = e->next;
                        list_del(&en->list);
                        points++;
                        enemigos.qtt_alive--;
                    }
                    // esquinas
                    else if (zeldo_data.y-1 == en->y &&
                        zeldo_data.x-1 == en->x) {
                        deleted = 1;
                        e = e->next;
                        list_del(&en->list);
                        points++;
                        enemigos.qtt_alive--;
                    }
                    else if (zeldo_data.y-1 == en->y &&
                        zeldo_data.x+1 == en->x) {
                        deleted = 1;
                        e = e->next;
                        list_del(&en->list);
                        points++;
                        enemigos.qtt_alive--;
                    }
                    else if (zeldo_data.y+1 == en->y &&
                        zeldo_data.x-1 == en->x) {
                        deleted = 1;
                        e = e->next;
                        list_del(&en->list);
                        points++;
                        enemigos.qtt_alive--;
                    }
                    else if (zeldo_data.y+1 == en->y &&
                        zeldo_data.x+1 == en->x) {
                        deleted = 1;
                        e = e->next;
                        list_del(&en->list);
                        points++;
                        enemigos.qtt_alive--;
                    }

                    if (deleted == 0) {
                        e = e->next;
                    }
                }
                break;
                }
            default:
                break;
        }

        // movimiento de los enemigos
        ++counter_movimiento;

        if (counter_movimiento%5 == 0)
        {
            struct list_head *e = list_first(&enemigos.enemigos);
            list_for_each(e, &enemigos.enemigos)
            {
                struct enemigo *en = list_head_to_enemigo_struct(e);
                int mov = rand()%4;

                switch (mov) {
                    case 0:
                        // arriba
                        if (en->y > 1 &&
                            tablero[((en->y-1)*80 + en->x)] != '#' &&
                            tablero[((en->y-1)*80 + en->x)] != '@') {
                            en->y--;
                        }
                        break;
                    case 1:
                        // izquierda
                        if (en->x > 0 &&
                            tablero[(en->y*80 + en->x-1)] != '#' &&
                            tablero[(en->y*80 + en->x-1)] != '@') {
                            en->x--;
                        }
                        break;
                    case 2:
                        // abajo
                        if (en->y < 24 &&
                            tablero[((en->y+1)*80 + en->x)] != '#' &&
                            tablero[((en->y+1)*80 + en->x)] != '@') {
                            en->y++;
                        }
                        break;
                    case 3:
                        // derecha
                        if (en->x < 79 &&
                            tablero[(en->y*80 + en->x+1)] != '#' &&
                            tablero[(en->y*80 + en->x+1)] != '@') {
                            en->x++;
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        // si zeldo está en la misma casilla que un enemigo, pierde un punto de vida
        struct list_head *e = list_first(&enemigos.enemigos);
        for (; e != &enemigos.enemigos;) {
            struct enemigo *en = list_head_to_enemigo_struct(e);
            if (zeldo_data.y == en->y &&
                zeldo_data.x == en->x) {
                zeldo_data.hp--;
                e = e->next;
                list_del(&en->list);
                enemigos.qtt_alive--;
            }
            else {
                e = e->next;
            }
        }

        render(tablero, points, zeldo_data, &enemigos);

        if (enemigos.qtt_alive == 0 || zeldo_data.hp == 0) {
            break;
        }
    }

    clrscr(0);

    goto_xy(0, 0);

    change_color(0x02, 0x00);

    if (zeldo_data.hp == 0) {
        write(1, "Has perdido.\n\n", 14);
    }
    else {
        write(1, "Has ganado.\n\n", 13);
    }

    write(1, "De repente Zeldo escucha un ruido raro desde el cielo.\n\n", 56);

    write(1, "Zeldo: Que es ese ruido?\n\n", 26);
    
    write(1, "Zeldo mira hacia arriba y ve un pajaro metalico gigante que se dirige hacia el y del que caen varias cuerdas.\n\n", 111);

    write(1, "Se abre una puerta en el pajaro y bajan varios hombres con raras espadas y le apuntan con ellas.\n\n", 98);

    write(1, "Zeldo: Quienes sois vosotros?\n\n", 31);

    write(1, "Hombre: Somos los abogados de Nintendo y te vamos a demandar por plagio.\n\n", 74);

    write(1, "Zeldo: Pero si este juego no tiene nada que ver con Zelda.\n\n", 60);

    write(1, "Abogado: No importa, te vamos a demandar igual.\n\n", 49);

    write(1, "Zeldo recibe un golpe en la cabeza y se desmaya. Despierta varias horas despues en un juzgado y acaba condenado a cadena perpetua por plagio a Nintendo.\n\n", 154);

    write(1, "FIN\n", 4);
}
