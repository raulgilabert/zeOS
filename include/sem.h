#include <list.h>

#define MAX_SEMAFOROS 10

typedef struct sem_data {
  int value;
  struct task_struct *owner;
  struct list_head blocked;
} sem_t;

sem_t* sem_create(int initial_value);

int semWait(sem_t* s);

int semSignal(sem_t* s);

int semDestroy(sem_t* s);