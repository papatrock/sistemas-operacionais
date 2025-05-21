/*
Patrick Oliveira Lemes
GRR20211777
*/
#define _POSIX_C_SOURCE 199309L

#include "ppos_data.h"
#include "ppos.h"
#include "ppos_core.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <valgrind/valgrind.h>
#include <sys/time.h>
#include <signal.h>

#define STACKSIZE 64 * 1024

task_t MainTask, Dispatcher, *CurrentTask, *PreviousTask, *ReadyQueue, *SuspendedQueue, *FinishedQueue;

int idCounter, userTasks;
unsigned int initial_processor_time, system_time = 0; // Contao o tempo do sistema

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action;

// estrutura de inicialização do timer
struct itimerval timer;

/**
 * Inicializa o sistema operaconal; deve ser chamado no inicio do main()
 */
void ppos_init()
{

  // desativa o buffer do printf
  setvbuf(stdout, 0, _IONBF, 0);

  // inicializa MainTask
  idCounter = 0;
  MainTask.id = idCounter; // main context sempre é 0;
  MainTask.status = RODANDO;
  MainTask.next = NULL;
  MainTask.prev = NULL;
  MainTask.systemProcess = 1; // Main é um processo do sistema
  MainTask.quantum = 20;      // desnecessario, só ñ pode ser 0
  MainTask.init_time = systime();
  MainTask.activations = 0;
  MainTask.processor_time = 0;
  CurrentTask = &MainTask;
  MainTask.awaiting_queue = NULL;


  //  cria dispatcher
  task_init(&Dispatcher, dispatcher, NULL);
  Dispatcher.systemProcess = 1; // seta o dispatcher como processo do systema
  Dispatcher.quantum = 20;
  userTasks = 0;

  // ajusta valores do temporizador
  timer.it_value.tv_usec = 1000;    // primeiro disparo, em micro-segundos
  timer.it_value.tv_sec = 0;        // primeiro disparo, em segundos
  timer.it_interval.tv_usec = 1000; // disparos subsequentes, em micro-segundos
  timer.it_interval.tv_sec = 0;     // disparos subsequentes, em segundos
  // registra a ação para o sinal de timer SIGALRM (sinal do timer)
  action.sa_handler = tick_handler;
  sigemptyset(&action.sa_mask); // limpa a máscara de sinais
  action.sa_flags = 0;          // sem flags adicionais

  // registra a ação para o sinal de timer SIGALRM
  if (sigaction(SIGALRM, &action, NULL) < 0)
  {
    perror("Erro em sigaction");
    exit(1);
  }

  // arma o temporizador ITIMER_REAL
  if (setitimer(ITIMER_REAL, &timer, 0) < 0)
  {
    perror("Erro em setitimer: ");
    exit(1);
  }
}

/*
  Ao receber um tick, decrementa o quantum do processo (se for um processo de usuario)
*/
void tick_handler()
{
  system_time++; //+ 1000 microsegundo
  if (!CurrentTask->systemProcess)
    CurrentTask->quantum--;
  if (CurrentTask->quantum <= 0)
  {
#ifdef DEBUG
    printf("acabou quantum da task: %d, trocando de task....\n", CurrentTask->id);
#endif
    task_yield();
  }
}

unsigned int systime()
{
  return system_time;
}

void print_elem(void *ptr)
{
  task_t *elem = ptr;

  if (!elem)
    return;

  elem->prev ? printf("%d", elem->prev->id) : printf("*");
  printf("<%d>", elem->id);
  elem->next ? printf("%d", elem->next->id) : printf("*");
}

/*
-20 max prio
    0 default prio
+20 min prio
cada vez que tiver uma troca de contexto, a tarefa escolhida volta para
prioridade estática e as não escolhidas envelhecem em -1
*/
task_t *scheduler()
{

  if (queue_size((queue_t *)ReadyQueue) > 0)
  {
    task_t *aux = ReadyQueue->next;
    task_t *maxPrio = ReadyQueue;
    do
    {
      if (aux->id != 0)
        if (aux->agingPrio < maxPrio->agingPrio)
          maxPrio = aux;

    } while ((aux = aux->next) != ReadyQueue);

    // Envelhece todas menos a escolhida
    aux = ReadyQueue;
    do
    {
      if (aux != maxPrio && aux->id != 0)
      {
        if (aux->agingPrio > -20)
          aux->agingPrio--;
      }
      aux = aux->next;
    } while (aux != ReadyQueue);

    // Reset da prioridade dinâmica da escolhida
    maxPrio->agingPrio = maxPrio->staticPrio;
    return maxPrio;
  }
  return NULL;
}

/*
Ajusta prioridade estática
*/
void task_setprio(task_t *task, int prio)
{
  if (task)
  {
    task->staticPrio = prio;
    task->agingPrio = prio;
  }
  else
  {
    CurrentTask->staticPrio = prio;
    CurrentTask->agingPrio = prio;
  }
}

/*
retorna o valor da prioridade estática da tarefa
*/
int task_getprio(task_t *task)
{

  if (task)
    return task->staticPrio;

  return CurrentTask->staticPrio;
}

/*
Para a execução da tarefa atual e retorna ao dispatcher

*/
void dispatcher()
{
  queue_remove((queue_t **)&ReadyQueue, (queue_t *)&Dispatcher);

  Dispatcher.status = RODANDO;

  while (userTasks > 0)
  {
#ifdef DEBUG
    printf("USER TASKS:%d\n", userTasks);
#endif
    // escolhe proxima tarefa (fila com prioridade)
    task_t *nextTask = scheduler();
    // se a main for o primeiro da fila e ainda tiver tarefas de usuario, passa pro proximo da fila
    if (nextTask->id == 0)
    {
      nextTask = nextTask->next;
    }

    if (nextTask)
    {
#ifdef DEBUG
      printf("Tarefa atual: %d\n user tasks:%d \n", nextTask->id, userTasks);
#endif
      queue_remove((queue_t **)&ReadyQueue, (queue_t *)nextTask);
      nextTask->quantum = 20;

      initial_processor_time = systime();
      task_switch(nextTask);
      nextTask->processor_time += (systime() - initial_processor_time);

      //   ao voltar pro dispatcher:
      switch (nextTask->status)
      {
      case RODANDO:
        nextTask->status = PRONTA;
        queue_append((queue_t **)&ReadyQueue, (queue_t *)nextTask);
        break;

      case SUSPENSA: // nao usado ainda
        queue_append((queue_t **)&SuspendedQueue, (queue_t *)nextTask);
        break;

      case TERMINADA:
        queue_append((queue_t **)&FinishedQueue, (queue_t *)nextTask);
        userTasks--;
        break;

      default:
        break;
      }
    }
  }
  task_exit(0);
}

void task_exit(int exit_code)
{

  // exit na main, passa pro dispatcher
  if (CurrentTask->id == 0)
  {
    CurrentTask->activations++; // conta a ativação fianl do dispatcher (para ficar parecido com a saida esperada)
    printf("Task %d exit: execution time %4d ms, processor time %4d ms, %d activations\n", CurrentTask->id, systime() - CurrentTask->init_time, CurrentTask->processor_time, CurrentTask->activations);
    task_yield();

    // fim do dispatcher
#ifdef DEBUG
    printf("Passou do task_yield no exit\n");
#endif
    task_switch(&MainTask);
  }
  // exit no dispatcher, passa o controle pra main
  else if (CurrentTask->id == 1)
  {
    // TODO limpa pilhas
    queue_append((queue_t **)&FinishedQueue, (queue_t *)&Dispatcher);
    freeQueueTasks(ReadyQueue);
    freeQueueTasks(FinishedQueue);
    freeQueueTasks(SuspendedQueue);
#ifdef DEBUG
    printf("chegou no exit do dispatcher, retorna pra main e finaliza\n");
#endif
    printf("Task %d exit: execution time %4d ms, processor time %4d ms, %d activations\n", CurrentTask->id, systime() - CurrentTask->init_time, CurrentTask->processor_time, CurrentTask->activations);
    task_switch(&MainTask);
  }
  // fim de uma task, retira da fila de prontas ou <suspensas> e coloca na fila de finalizadas
  // Se tiver tarefas suspensas esperando pela task, libera elas
  else
  {
#ifdef DEBUG
    printf("EXIT DE TAREFA, seta status como terminada\n");
#endif
    CurrentTask->status = TERMINADA;
    CurrentTask->processor_time = CurrentTask->processor_time + (20 - CurrentTask->quantum); // quanto do quantum utilizou antes de terminar
    printf("Task %d exit: execution time %4d ms, processor time %4d ms, %d activations\n", CurrentTask->id, systime() - CurrentTask->init_time, CurrentTask->processor_time, CurrentTask->activations);
    // passa o controle pro dispatcher
    task_yield();
  }

  // perror("Erro no task_exit");
  // exit(1);
}

void task_yield()
{

  if (CurrentTask->status != TERMINADA)
  {
    CurrentTask->status = PRONTA;
    queue_append((queue_t **)&ReadyQueue, (queue_t *)CurrentTask);
  }
  task_switch(&Dispatcher);
}

/**
 * Alterna a execução para a tarefa indicada
 * @param *task
 * return ID?
 */
int task_switch(task_t *task)
{
  if (!task)
    return -1;

  PreviousTask = CurrentTask;

  CurrentTask = task;
  CurrentTask->status = RODANDO;
  CurrentTask->activations++;

  // troca de contexto
  // começa a conta o tempo de processamento

  swapcontext(&PreviousTask->context, &CurrentTask->context);

  return 0;
}

/**
 * Cria uma mensagem do tipo protocolo_t e a inicializa com os parametros
 *
 * @param task estrutura que referencia a tarefa a ser iniciada
 * @param start_routine função que será executada pela tarefa
 * @param arg parâmetro a passar para a tarefa que está sendo iniciada
 * @return ID > 0 ou erro
 */
int task_init(task_t *task, void (*start_routine)(void *), void *arg)
{

  getcontext(&task->context);

  char *stack = malloc(STACKSIZE);
  if (stack)
  {
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;
    task->vg_id = VALGRIND_STACK_REGISTER(task->context.uc_stack.ss_sp, task->context.uc_stack.ss_sp + STACKSIZE);
  }
  else
  {
    perror("Erro na criação da pilha: ");
    return -1;
  }

  task->next = NULL;
  task->prev = NULL;
  task->status = 0;
  idCounter++;
  task->id = idCounter;
  task->staticPrio = 0; // Prioridade default
  task->agingPrio = 0;
  task->systemProcess = 0; // processo de usuario (default)
  task->init_time = systime();
  task->activations = 0;
  task->processor_time = 0;
  task->awaiting_queue = NULL;
  // coloca a task no fim da fila de prontas
  // TODO tratar erros de fila
  queue_append((queue_t **)&ReadyQueue, (queue_t *)task);

#ifdef DEBUG
  printf("\nQUEUE SIZE: %d\n", queue_size((queue_t *)ReadyQueue));
  queue_print("Saida gerada  ", (queue_t *)ReadyQueue, print_elem);
#endif

  // TODO como saber quantos arguemtnos
  makecontext(&task->context, (void *)start_routine, 1, arg);

  if (idCounter > 1)
    userTasks++;

  return task->id;
}

/**
 * Termina a tarefa corrente com um status de encerramento
 * @param exit_code (nao sera usado ainda)
 */

int task_id()
{
  return CurrentTask->id;
}

void task_suspend (task_t **queue){

}

void task_awake (task_t * task, task_t **queue){

}

int task_wait (task_t *task){
  
  queue_append((queue_t**)task,(queue_t*)CurrentTask);
  
  printf("Teste: %d\n",task->next->id);

  //task->awaiting_queue = (queue_t*) CurrentTask; // tarefa atual aguarda task terminar
  
  return 0;
}

void freeQueueTasks(task_t *queue)
{
  while (queue)
  {
    task_t *elem = queue;
    queue_remove((queue_t **)&queue, (queue_t *)queue);
    if (elem->id != 0)
    {
      if (elem->context.uc_stack.ss_sp)
      {
        free(elem->context.uc_stack.ss_sp);
        // dezfaz o registro da pilha no valgrind
        VALGRIND_STACK_DEREGISTER(elem->vg_id);
      }
    }
  }
}