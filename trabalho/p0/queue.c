#include "queue.h"
#include <stdio.h>


int queue_size(queue_t *queue){
    
    //fila vazia
    if(!queue)
        return 0;

    int count = 1;

    for(queue_t *aux = queue; aux->next != queue; aux = aux->next)
        count++;

    return count;
}

void queue_print(char *name, queue_t *queue, void print_elem(void*)){

    if(!queue)
        return;

    for(queue_t *aux = queue; aux->next != aux; aux = aux->next)
        print_elem(queue);
}

int queue_append (queue_t **queue, queue_t *elem){
    
    //fila nao existe
    if(!queue)  //*queue?
        return -1;

    //elemento nao existe
    if(!elem)
        return -2;

    //elemento pertence a outra fila
    if(elem->next != NULL || elem->prev != NULL)
        return -3;

    // fila vazia
    if(queue_size(*queue) == 0){
        
        elem->next = elem;
        elem->prev = elem;  
        *queue = elem;

        printf("Depois da atribuição: elem->next = %p, elem->prev = %p\n", (void*)(*queue)->next, (void*)(*queue)->prev);


        return 0;
    }

    queue_t *aux = (*queue)->prev;
    aux->next = elem;
    elem->next = *queue;
    elem->prev = aux;

    aux->next = elem;
    (*queue)->prev = elem;

    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem){

    if(!queue)  //*queue?
        return -1;

    //elemento nao existe
    if(!elem)
        return -2;

    if(queue_size(*queue) == 0)
        return 0;

    queue_t *aux = *queue;
    while(aux->next != aux)
            aux = aux->next;
    
    queue_t *prev = aux->prev;
    prev->next = aux->next;
    aux->next->prev = prev;

    aux->next = NULL;
    aux->prev = NULL;

    return 0;
}

