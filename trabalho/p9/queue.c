/*

Nome: Patrick Oliveira Lemes

GRR: 20211777

*/

#include "queue.h"

int queue_size(queue_t *queue)
{
  if (!queue)
    return 0;

  int count = 1;
  queue_t *aux = queue;

  while (aux->next && aux->next != queue)
  {
    count++;
    aux = aux->next;
  }

  return count;
}

void queue_print(char *name, queue_t *queue, void print_elem(void *))
{

  if (queue)
  {
    queue_t *aux = queue;
    do
    {
      print_elem(aux);
      aux = aux->next;
    } while (aux != queue);
  }
}

int queue_append(queue_t **queue, queue_t *elem)
{

  // fila nao existe
  if (!queue)
    return -1;

  // elemento nao existe
  if (!elem)
    return -2;

  // elemento pertence a outra fila
  if (elem->next != NULL || elem->prev != NULL)
    return -3;

  // fila vazia
  if (queue_size(*queue) == 0)
  {

    elem->next = elem;
    elem->prev = elem;
    *queue = elem;

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

int queue_remove(queue_t **queue, queue_t *elem)
{

  if (!queue)
    return -1;

  // elemento nao existe
  if (!elem)
    return -2;

  // fila vazia
  int size = queue_size(*queue);
  if (size == 0)
    return 0;

  queue_t *aux = *queue;
  // percorre toda a fila até achar o elem ou chegar no fim
  while (aux != elem && aux->next != *queue)
    aux = aux->next;

  // elem nao pertence a fila
  if (aux != elem)
    return -3;

  // fila apenas com um elemento e esse elemento é elem
  if (size == 1)
  {
    aux->next = NULL;
    aux->prev = NULL;
    *queue = NULL;
    return 0;
  }

  aux->prev->next = aux->next;
  aux->next->prev = aux->prev;

  // elem é o primeiro da fila
  if (*queue == aux)
    *queue = aux->next;

  aux->next = NULL;
  aux->prev = NULL;

  return 0;
}
