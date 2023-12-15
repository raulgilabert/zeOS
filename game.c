#include <libc.h>
#include <sem.h>
#include <game.h>

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
    int en_pantalla;
    int dead;
};

struct enemigos_data {
    int qtt_alive;
    int qtt;
    struct enemigo enemigos[20];
};

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

    clrscr(pantalla);

    goto_xy(zeldo_data.x, zeldo_data.y);
    change_color(0x00, 0x02);
    write(1, "Z", 1);

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

    for (int i = 0; i < enemigos->qtt; ++i) {
        if (enemigos->enemigos[i].dead == 0 && enemigos->enemigos[i].en_pantalla == 1) {
            goto_xy(enemigos->enemigos[i].x, enemigos->enemigos[i].y);

            change_color(0x01, 0x02);
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

    write(1, "Los enemigos se mueven por la pantalla de forma aleatoria cada 4 movimientos de Zeldo.\n", 87);
    write(1, "Si Zeldo se coloca en la misma casilla que un enemigo, este le atacara y morira al instante.\n", 93);
    write(1, "Si Zeldo se queda sin vida, el juego termina.\n", 46);

    write(1, "\n", 1);
    do {
        write(1, "Pulsa [Enter] para continuar\n", 29);
        semSignal(sem_keyboard);
        semWait(sem_game);
    } while (d.l != '\n');

    char tablero[80*25];

    // Generación del tablero
    for (int i = 0; i < 80*25; ++i) {
        tablero[i] = ' ';
    }

    // generacion aleatoria de arbustos
    for (int i = 0; i < 100; ++i) {
        int x = rand()%80;
        int y = rand()%25;

        if (tablero[(y*80 + x)] == ' ') {
            tablero[(y*80 + x)] = '#';
        }
    }

    // generacion aleatoria de piedras
    for (int i = 0; i < 50; ++i) {
        int x = rand()%80;
        int y = rand()%25;

        if (tablero[(y*80 + x)] == ' ') {
            tablero[(y*80 + x)] = '@';
        }
    }


    // Generación de enemigos
    struct enemigos_data *enemigos = (struct enemigos_data *)memRegGet(1); // una página permite hasta 127 enemigos y el struct tiene 20

    enemigos->qtt = enemigos->qtt_alive = 20;
    for (int i = 0; i < enemigos->qtt; ++i) {
        do {
        enemigos->enemigos[i].x = 1 + rand()%78;
        enemigos->enemigos[i].y = 1 + rand()%23;

        } while (tablero[(enemigos->enemigos[i].y*80 + enemigos->enemigos[i].x)] != ' ');
        enemigos->enemigos[i].en_pantalla = 1;
        enemigos->enemigos[i].dead = 0;
    }

    // Generación de Zeldo
    struct zeldo zeldo_data = {40, 11, 3};

    int points = 0;

    render(tablero, points, zeldo_data, enemigos);

    int counter_movimiento = 0;

    while (1)
    {
        semSignal(sem_keyboard);
        semWait(sem_game);

        // movimiento de zeldo y ataque
        switch (d.l) {
            case 'w':
                // arriba
                if (zeldo_data.y > 0 &&
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
            case ' ':

                for (int i = 0; i < enemigos->qtt; ++i) {
                    if (enemigos->enemigos[i].dead == 0 && enemigos->enemigos[i].en_pantalla == 1) {
                        // comprobamos en las 8 direcciones de Zeldo
                        int distancia_x = zeldo_data.x - enemigos->enemigos[i].x;
                        int distancia_y = zeldo_data.y - enemigos->enemigos[i].y;

                        if (distancia_x >= -1 && distancia_x <= 1 &&
                            distancia_y >= -1 && distancia_y <= 1) {
                            enemigos->enemigos[i].dead = 1;
                            points++;
                            enemigos->qtt_alive--;

                            write(1, "a", 1);
                        }
                    }
                }
                break;
            default:
                break;
        }

        // movimiento de los enemigos
        ++counter_movimiento;

        if (counter_movimiento%5 == 0)
        {
            for (int i = 0; i < enemigos->qtt; ++i) {
                if (enemigos->enemigos[i].dead == 0) {
                    int direccion = rand()%4;

                    switch (direccion) {
                        case 0:
                            // arriba
                            if (enemigos->enemigos[i].y > 0 &&
                                tablero[((enemigos->enemigos[i].y-1)*80 + enemigos->enemigos[i].x)] != '#' &&
                                tablero[((enemigos->enemigos[i].y-1)*80 + enemigos->enemigos[i].x)] != '@') {
                                enemigos->enemigos[i].y--;
                            }
                            break;
                        case 1:
                            // izquierda
                            if (enemigos->enemigos[i].x > 0 &&
                                tablero[(enemigos->enemigos[i].y*80 + enemigos->enemigos[i].x-1)] != '#' &&
                                tablero[(enemigos->enemigos[i].y*80 + enemigos->enemigos[i].x-1)] != '@') {
                                enemigos->enemigos[i].x--;
                            }
                            break;
                        case 2:
                            // abajo
                            if (enemigos->enemigos[i].y < 24 &&
                                tablero[((enemigos->enemigos[i].y+1)*80 + enemigos->enemigos[i].x)] != '#' &&
                                tablero[((enemigos->enemigos[i].y+1)*80 + enemigos->enemigos[i].x)] != '@') {
                                enemigos->enemigos[i].y++;
                            }
                            break;
                        case 3:
                            // derecha
                            if (enemigos->enemigos[i].x < 79 &&
                                tablero[(enemigos->enemigos[i].y*80 + enemigos->enemigos[i].x+1)] != '#' &&
                                tablero[(enemigos->enemigos[i].y*80 + enemigos->enemigos[i].x+1)] != '@') {
                                enemigos->enemigos[i].x++;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        // si zeldo está en la misma casilla que un enemigo, pierde un punto de vida
        for (int i = 0; i < enemigos->qtt; ++i) {
            if (enemigos->enemigos[i].dead == 0 && enemigos->enemigos[i].en_pantalla == 1) {
                if (zeldo_data.x == enemigos->enemigos[i].x &&
                    zeldo_data.y == enemigos->enemigos[i].y) {
                    zeldo_data.hp--;

                    // además, el enemigo desaparece
                    enemigos->enemigos[i].en_pantalla = 0;
                    enemigos->qtt_alive--;
                }
            }
        }

        render(tablero, points, zeldo_data, enemigos);

        if (enemigos->qtt_alive == 0 || zeldo_data.hp == 0) {
            break;
        }
    }

    clrscr(0);

    goto_xy(0, 0);

    if (zeldo_data.hp == 0) {
        write(1, "Has perdido.\n\n", 14);
    }
    else {
        write(1, "Has ganado.\n\n", 13);
    }

    write(1, "De repente Zeldo escucha un ruido raro desde el cielo.\n\n", 58);

    write(1, "Zeldo: Que es ese ruido?\n\n", 26);
    
    write(1, "Zeldo mira hacia arriba y ve un pajaro metalico gigante que se dirige hacia el y del que caen varias cuerdas.\n\n", 111);

    write(1, "Se abre una puerta en el pajaro y bajan varios hombres con raras espadas y le apuntan con ellas.\n\n", 98);

    write(1, "Zeldo: Quienes sois vosotros?\n\n", 31);

    write(1, "Hombre: Somos los abogados de Nintendo y te vamos a demandar por plagio.\n\n", 74);

    write(1, "Zeldo: Pero si este juego no tiene nada que ver con Zelda.\n\n", 60);

    write(1, "Abogado: No importa, te vamos a demandar igual.\n\n", 49);

    write(1, "Zeldo recibe un golpe en la cabeza y se desmaya. Despierta varias horas despues en un juzgado y acaba condenado a cadena perpetua por plagio a Nintendo.\n", 154);

    write(1, "FIN\n", 4);
}