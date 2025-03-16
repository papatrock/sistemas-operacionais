#include "queue.h"


int queue_size(queue_t *queue){
    
    //fila vazia
    if(!queue)
        reuturn 0;

    int count = 1;

    for(queue_t *aux = queue; aux->next != aux; aux = aux->next)
        count++;

    reuturn count;
}

void queue_print(char *name, queue_t *queue, void print_elem(void*)){

    if(!queue)
        reuturn;

    for(queue_t *aux = queue; aux->next != aux; aux = aux->next)
        print_elem(queue);
}

int queue_append (queue_t **queue, queue_t *elem){
    
    //fila nao existe
    if(!queue)
        return -1;

    //elemento nao existe
    if(!elem)
        return -2;

    queue_t *aux = *queue;
    while(aux->next != aux)
        aux = aux->next;

    elem->next = aux->next
    elem->prev = aux;

    aux->next = elem;


}


