#ifndef PPOS_CORE_H
#define PPOS_CORE_H

#define _POSIX_C_SOURCE 199309L

#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

#define TERMINADA -1
#define SUSPENSA 0
#define PRONTA 1
#define RODANDO 2

// Variáveis globais
extern task_t MainTask, Dispatcher, *CurrentTask, *PreviousTask, *ReadyQueue, *SuspendedQueue, *FinishedQueue;
extern int idCounter, userTasks;
extern struct sigaction action;
extern struct itimerval timer;

/**
 * Inicializa o sistema operacional.
 */
void ppos_init();

/**
 * Função do dispatcher, responsável por gerenciar as tarefas.
 */
void dispatcher();

/**
 * Tratador de tick, aqui é incrementado o relogio do sistema
 */
void tick_handler();

/**
 * Retorna o tempo do sistema
 */
unsigned int systime();

/**
 * Imprime os elementos de uma fila.
 * @param ptr Ponteiro para o elemento a ser impresso.
 */
void print_elem(void *ptr);

/**
 * Escolhe a próxima tarefa a ser executada.
 * @return Ponteiro para a tarefa escolhida.
 */
task_t *scheduler();

/**
 * Define a prioridade de uma tarefa.
 * @param task Ponteiro para a tarefa.
 * @param prio Prioridade a ser definida.
 */
void task_setprio(task_t *task, int prio);

/**
 * Retorna a prioridade de uma tarefa.
 * @param task Ponteiro para a tarefa.
 * @return Prioridade da tarefa.
 */
int task_getprio(task_t *task);

/**
 * Finaliza a tarefa atual.
 * @param exit_code Código de saída da tarefa.
 */
void task_exit(int exit_code);

/**
 * Faz a tarefa atual ceder a CPU.
 */
void task_yield();

/**
 * Alterna a execução para a tarefa indicada.
 * @param task Ponteiro para a tarefa a ser executada.
 * @return 0 em caso de sucesso, -1 em caso de erro.
 */
int task_switch(task_t *task);

/**
 * Inicializa uma tarefa.
 * @param task Ponteiro para a estrutura da tarefa.
 * @param start_routine Função que será executada pela tarefa.
 * @param arg Argumento a ser passado para a função.
 * @return ID da tarefa criada ou -1 em caso de erro.
 */
int task_init(task_t *task, void (*start_routine)(void *), void *arg);

/**
 * Retorna o ID da tarefa atual.
 * @return ID da tarefa atual.
 */
int task_id();

/**
 * Libera as tarefas de uma fila.
 * @param queue Ponteiro para a fila de tarefas.
 */
void freeQueueTasks(task_t *queue);

int task_wait (task_t *task);

/*
    retira a tarefa atual da fila de tarefas prontas (se estiver nela);
    ajusta o status da tarefa atual para “suspensa”;
    insere a tarefa atual na fila apontada por queue (se essa fila não for nula);
    retorna ao dispatcher.
*/
void task_suspend (task_t **queue) ;

/*
    Acorda uma tarefa que está suspensa em uma dada fila, através das seguintes ações:
    se a fila queue não for nula, retira a tarefa apontada por task dessa fila;
    ajusta o status dessa tarefa para “pronta”;
    insere a tarefa na fila de tarefas prontas;
    continua a tarefa atual (não retorna ao dispatcher)

*/

#endif // PPOS_CORE_H