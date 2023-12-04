#include <circ_buff.h>

void cb_init(struct circ_buff *cb)
{
    for (int i = 0; i < CIRC_BUFF_SIZE; ++i)
    {
        cb->buff[i] = 0;
    }
    cb->keyboard = 0;
    cb->user = 0;
} 

char cb_next(struct circ_buff *cb)
{
    char a = cb->buff[cb->user++];
    cb->user = (cb->user)%CIRC_BUFF_SIZE;
    return a;
}

int cb_empty(struct circ_buff *cb)
{
    if(cb->keyboard != cb->user)
        return 0;
    else
        return 1;

}

int cb_add(struct circ_buff *cb, char c)
{
    if ((CIRC_BUFF_SIZE + (cb->user - cb->keyboard))%CIRC_BUFF_SIZE != 1)
    {
        cb->buff[cb->keyboard++] = c;
        cb->keyboard = (cb->keyboard)%CIRC_BUFF_SIZE;
    }
}
