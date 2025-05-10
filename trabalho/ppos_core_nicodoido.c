#include "ppos.h"
#include "queue.h"
#include "ppos_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <ucontext.h>

#define STACKSIZE 64 * 1024
#define QUANTA 20
#define USERTASK 0
#define KERNELTASK 1
#define STATUS_DONE 2
#define STATUS_READY 3
#define STATUS_SUSPENDED 4
// #define TASKDEBUG

#define ALPHA -1

task_t *currT;
task_t mainT, dispatcherT;
int idCounter = 0, userTasksCounter = 0;

queue_t *readyQueues;

struct sigaction action;

struct itimerval timer;

void ppos_init()
{

#ifdef DEBUG
  printf("DEBUG: starting main task\n");
#endif

  setvbuf(stdout, 0, _IONBF, 0);
  readyQueues = NULL;

  mainT.id = 0;
  mainT.quantum = QUANTA;

  currT = &mainT;
  queue_append(&readyQueues, (queue_t *)&mainT);
  userTasksCounter++;

#ifdef DEBUG
  printf("DEBUG: main task started successfully\n");
#endif

#ifdef DEBUG
  printf("DEBUG: starting dispatcher task\n");
#endif

  task_init(&dispatcherT, &dispatcher, NULL);
  dispatcherT.type = KERNELTASK;
  task_setprio(&mainT, 0);

#ifdef DEBUG
  printf("DEBUG: dispatcher task started successfully\n");
#endif

  initalize_handler();
  initalize_timer();

  return;
}

int task_init(task_t *task, void (*start_routine)(void *), void *arg)
{

#ifdef DEBUG
  printf("DEBUG: starting new task\n");
#endif

  getcontext(&task->context);

  char *stack = malloc(STACKSIZE);
  if (stack)
  {
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;
#ifdef DEBUG
    printf("DEBUG: task stack malloc completed \n");
#endif
  }
  else
  {
    perror("Erro na criação da pilha: ");
    exit(1);
  }
#ifdef DEBUG
  printf("DEBUG: creating context for the new task\n");
#endif
  makecontext(&task->context, (void *)(*start_routine), 1, arg);

  task->id = ++idCounter;

  if (idCounter > 1)
  {
    queue_append(&readyQueues, (queue_t *)task);
    userTasksCounter++;
    task->status = STATUS_READY;
    task->type = USERTASK;
  }
  task_setprio(task, 0);
  task->quantum = QUANTA;
#ifdef DEBUG
  printf("DEBUG: task #%d created successfully!\n", task->id);
#endif

  return task->id;
}

int task_id()
{
  return currT->id;
}

void task_exit(int exit_code)
{

  if (currT == &mainT)
  {
#ifdef DEBUG
    printf("DEBUG: giving control from main to dispatcher\n");
#endif
    task_switch(&dispatcherT);
  }

  if (currT == &dispatcherT)
  {
#ifdef DEBUG
    printf("DEBUG: finishing dispatcher and exiting the program\n");
#endif
    exit(exit_code);
  }

#ifdef DEBUG
  printf("DEBUG: finishing task #%d\n", currT->id);
#endif
  currT->status = STATUS_DONE;
  task_switch(&dispatcherT);
  return;
}

int task_switch(task_t *task)
{

  if (!task)
    return -1;

  task_t *prevT = currT;
  currT = task;

#ifdef DEBUG
  printf("DEBUG: switching tasks #%d with #%d\n", prevT->id, currT->id);
#endif

  return swapcontext(&prevT->context, &currT->context);
}

void dispatcher()
{

#ifdef DEBUG
  printf("DEBUG: running dispatcher\n");
#endif

  while (userTasksCounter > 0)
  {
    task_t *nextT = scheduler();
#ifdef DEBUG
    printf("DEBUG: next running task is #%d\n", nextT->id);
#endif

    if (nextT)
    {
      nextT->quantum = QUANTA;
      task_switch(nextT);

      switch (nextT->status)
      {
      case STATUS_READY:
        task_setprio(nextT, nextT->static_prio);
        break;

      case STATUS_DONE:
        queue_remove(&readyQueues, (queue_t *)nextT);
        userTasksCounter--;
        break;

      case STATUS_SUSPENDED:

        break;

      default:
        exit(-1);
      }
    }
  }
  task_exit(0);
}

task_t *scheduler()
{
  if (!readyQueues)
    return NULL;

  task_t *auxT = (task_t *)readyQueues;
  task_t *nextT = auxT;
  int min_prio = auxT->dynamic_prio;
  int tamReady = queue_size(readyQueues);
  for (int i = 0; i < tamReady; ++i)
  {
    if (auxT->dynamic_prio < min_prio)
    {
      nextT = auxT;
      min_prio = nextT->dynamic_prio;
    }
    auxT = auxT->next;
  }

  auxT = nextT;
  for (int i = 0; i < tamReady - 1; ++i)
  {
    auxT = auxT->next;
    if (auxT->dynamic_prio > -20)
      auxT->dynamic_prio--;
  }
#ifdef DEBUG
  printf("DEBUG: task #%d selected with priority %d\n", nextT->id, nextT->dynamic_prio);
#endif
  return (task_t *)nextT;
}

void task_yield()
{
#ifdef DEBUG
  printf("DEBUG: task #%d giving cpu control\n", currT->id);
#endif
  task_switch(&dispatcherT);
}

void task_setprio(task_t *task, int prio)
{
  if (prio > 20 || prio < -20)
    exit(-1);
  if (!task)
    currT->dynamic_prio = currT->static_prio = prio;

  task->dynamic_prio = task->static_prio = prio;
}

int task_getprio(task_t *task)
{
  if (!task)
    return currT->static_prio;

  return task->static_prio;
}

void tratador(int signum)
{
  if (currT->type == KERNELTASK)
    return;

  --currT->quantum;

  if (currT->quantum == 0)
    task_yield();
}

void initalize_handler()
{
  action.sa_handler = tratador;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  if (sigaction(SIGALRM, &action, 0) < 0)
  {
    perror("Erro em sigaction: ");
    exit(1);
  }
}

void initalize_timer()
{
  timer.it_value.tv_usec = 1000;    // primeiro disparo, em micro-segundos
  timer.it_interval.tv_usec = 1000; // disparos subsequentes, em micro-segundos

  if (setitimer(ITIMER_REAL, &timer, 0) < 0)
  {
    perror("Erro em setitimer: ");
    exit(1);
  }
}