#define CIRC_BUFF_SIZE 10

struct circ_buff
{
    char buff[CIRC_BUFF_SIZE];
    char keyboard; //escritura ==> max = user-1 posiciones de lo contrario no sabríamos si está lleno o 
    char user; //lectura ==> max = keyboard

};


void cb_init(struct circ_buff *cb);
char cb_next(struct circ_buff *b);
int cb_empty(struct circ_buff *b);
int cb_add(struct circ_buff *b, char c);


