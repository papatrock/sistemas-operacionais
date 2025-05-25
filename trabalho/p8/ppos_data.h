// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h> // biblioteca POSIX de trocas de contexto

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next; // ponteiros para usar em filas
  int id;                     // identificador da tarefa
  ucontext_t context;         // contexto armazenado da tarefa
  short status;               // pronta, rodando, suspensa, ...
  // ... (outros campos serão adicionados mais tarde)
  int vg_id;
  int staticPrio;
  int agingPrio;
  int systemProcess;
  int quantum;
  unsigned int init_time;
  int activations;
  unsigned int processor_time;
  struct task_t *awaiting_queue; // Fila que armazena tarefas aguardando
} task_t;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t;

#endif
