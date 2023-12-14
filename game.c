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

    char b;
    while (1) {
        semWait(d->sem_keyboard);
        wait_key(&b, -1);
        d->l = b;
        semSignal(d->sem_game);
    }
}

struct zeldo {
    int x;
    int y;
    int hp;
};

struct enemigo {
    int x;
    int y;
};

void game() {
    // inicialización

    sem_t *sem_keyboard = semCreate(0);
    sem_t *sem_game = semCreate(0);

    struct keyboard_thread_data d;
    d.sem_keyboard = sem_keyboard;
    d.sem_game = sem_game;

    int res = threadCreateWithStack(keyboard_thread, 1, &d);

    write(1, "la leyenda de zeldo\n", 20);

    write(1, "Controles:\n", 11);
    write(1, "  - WASD para moverse\n", 22);
    write(1, "  - [Espacio] para atacar\n", 26);

    do {
        write(1, "Pulsa [Enter] para continuar\n", 29);
        semSignal(sem_keyboard);
        semWait(sem_game);
    } while (d.l != '\n');

    char pantalla[25*80*2];

    for (int i = 0; i < 25*80*2; i+=2) {
        pantalla[i] = 0;
        pantalla[i+1] = 0x20;
    }

    // Generación aleatoria de arbustos
    int seed = gettime();
    int a = 16807;
    int b = 1103515245;
    int m = 2147483647;

    for (int i = 0; i < 150; ++i) {
        seed = (b * seed + a)%m;
        int x = seed%80;
        seed = (b * seed + a)%m;
        int y = seed%25;

        pantalla[2*(y*80 + x)] = '#';
        pantalla[2*(y*80 + x) + 1] = 0x2a;
    }

    // Generación aleatoria de rocas
    for (int i = 0; i < 100; ++i) {
        seed = (b * seed + a)%m;
        int x = seed%80;
        seed = (b * seed + a)%m;
        int y = seed%25;

        pantalla[2*(y*80 + x)] = '@';
        pantalla[2*(y*80 + x) + 1] = 0x26;
    }

    // Generación de Zeldo
    struct zeldo zeldo_data = {40, 13, 3};

    pantalla[2*(zeldo_data.y*80 + zeldo_data.x)] = 'Z';
    pantalla[2*(zeldo_data.y*80 + zeldo_data.x) + 1] = 0x20;

    clrscr(pantalla);


    while (1)
    {
        semSignal(sem_keyboard);
        semWait(sem_game);

        clrscr(pantalla);
        // limpiar casilla antigua de zeldo
        pantalla[2*(zeldo_data.y*80 + zeldo_data.x)] = 0;



        switch (d.l) {
            case 'w':
                write(1, "Arriba\n", 7);
                if (zeldo_data.y > 0) {
                    zeldo_data.y--;
                }
                break;
            case 'a':
                write(1, "Izquierda\n", 10);
                if (zeldo_data.x > 0) {
                    zeldo_data.x--;
                }
                break;
            case 's':
                write(1, "Abajo\n", 6);
                if (zeldo_data.y < 24) {
                    zeldo_data.y++;
                }
                break;
            case 'd':
                write(1, "Derecha\n", 8);
                if (zeldo_data.x < 79) {
                    zeldo_data.x++;
                }
                break;
            case ' ':
                // atacar
                break;
            default:
                write(1, "Tecla no reconocida\n", 21);
                break;
        }

        // dibujar zeldo
        pantalla[2*(zeldo_data.y*80 + zeldo_data.x)] = 'Z';



    }

}