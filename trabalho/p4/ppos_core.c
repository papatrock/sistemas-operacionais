/*
    Patrick Oliveira Lemes
    GRR20211777
*/

#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <valgrind/valgrind.h>

#define STACKSIZE 64*1024

task_t MainTask, Dispatcher, *CurrentTask,*PreviousTask,*ReadyQueue, *SuspendedQueue, *FinishedQueue;

int idCounter, userTasks;


void print_elem (void *ptr)
{
   task_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
}

/*
    -20 max prio
      0 default prio
    +20 min prio
    cada vez que tiver uma troca de contexto, a tarefa escolhida volta para
    prioridade estática e as não escolhidas envelhecem em -1
*/

task_t *scheduler(){
    
    if(queue_size((queue_t*)ReadyQueue) > 0){
        task_t *aux = ReadyQueue->next;
        task_t *maxPrio = ReadyQueue;
        do{
            if(aux->id != 0){
                if(aux->agingPrio < maxPrio->agingPrio){
                    maxPrio->agingPrio--; //envelhece a tarefa
                    maxPrio = aux;
                }else{
                    aux->agingPrio--; //evelhece a tarefa
                }
            }
        }while((aux = aux->next) != ReadyQueue);

        maxPrio->agingPrio = maxPrio->staticPrio;
        return maxPrio;
    }
    return NULL;
}

/*
    Ajusta prioridade estática
*/
void task_setprio(task_t *task, int prio){
    if(task){
        task->staticPrio = prio;
        task->agingPrio = prio;
    }
    else{
        CurrentTask->staticPrio = prio;
        CurrentTask->agingPrio = prio;
    }
}


/*
    retorna o valor da prioridade estática da tarefa
*/
int task_getprio(task_t *task){

    if(task)
        return task->staticPrio;
    
    return CurrentTask->staticPrio;   
}

/*
Para a execução da tarefa atual e retorna ao dispatcher

*/
void dispatcher(){
    queue_remove((queue_t**)&ReadyQueue,(queue_t*)&Dispatcher);

    Dispatcher.status = RODANDO;

    while(userTasks > 0){
        #ifdef DEBUG
        printf("USER TASKS:%d\n",userTasks);
        #endif

        // escolhe proxima tarefa (fila com prioridade)
        task_t *nextTask = scheduler();
        //se a main for o primeiro da fila e ainda tiver tarefas de usuario, passa pro proximo da fila
        if(nextTask->id == 0){
            nextTask = nextTask->next;
        }

        if(nextTask){

            #ifdef DEBUG
            printf("Tarefa atual: %d\n user tasks:%d \n",nextTask->id,userTasks);
            #endif
            
            queue_remove((queue_t**)&ReadyQueue, (queue_t*)nextTask);
            task_switch(nextTask);
           
            switch (nextTask->status)
            {
                case RODANDO:
                nextTask->status = PRONTA;
                queue_append((queue_t**)&ReadyQueue, (queue_t*)nextTask);
                break;

            case SUSPENSA: //nao usado ainda
                queue_append((queue_t**)&SuspendedQueue, (queue_t*)nextTask);
                break;

            case TERMINADA:
                queue_append((queue_t**)&FinishedQueue, (queue_t*)nextTask);
                userTasks--;
                break;
            
                default:
                break;
            }
        } 
    }
    task_exit(0);
    
}

/**
 * Inicializa o sistema operaconal; deve ser chamado no inicio do main()
 */
void ppos_init(){
    
    //desativa o buffer do printf
    setvbuf (stdout, 0, _IONBF, 0) ;
    
    //inicializa MainTask
    idCounter = 0;
    MainTask.id = idCounter; //main context sempre é 0;
    MainTask.status = RODANDO;
    MainTask.next = NULL;
    MainTask.prev = NULL;
    CurrentTask = &MainTask;
    
    //inicializa filas de tarefas (priontas, suspensa e finalizadas)
    // cria dispatcher
    
    task_init(&Dispatcher,dispatcher,NULL);
    
    userTasks = 0;
    
}

void task_exit (int exit_code){
    
    //exit na main, passa pro dispatcher
    if(CurrentTask->id == 0){
        task_yield();

        //fim do dispatcher
        #ifdef DEBUG
        printf("Passou do task_yield no exit\n");
        #endif
        task_switch(&MainTask);

    }
    //exit no dispatcher, passa o controle pra main
    else if(CurrentTask->id == 1){
        //TODO limpa pilhas
        queue_append((queue_t**)&FinishedQueue,(queue_t*)&Dispatcher);
        freeQueueTasks(ReadyQueue);
        freeQueueTasks(FinishedQueue);
        freeQueueTasks(SuspendedQueue);
        #ifdef DEBUG
        printf("chegou no exit do dispatcher, retorna pra main e finaliza\n");
        #endif
        task_switch(&MainTask);
        
    }
    // fim de uma task, retira da fila de prontas ou <suspensas> e coloca na fila de finalizadas
    else{
        #ifdef DEBUG
        printf("EXIT DE TAREFA, seta status como terminada\n");
        #endif
        CurrentTask->status = TERMINADA;
        //passa o controle pro dispatcher
        task_yield();
    }
    
    //perror("Erro no task_exit");
    //exit(1);
}

void task_yield(){

    if (CurrentTask->status != TERMINADA) {
        CurrentTask->status = PRONTA;
        queue_append((queue_t**)&ReadyQueue, (queue_t*)CurrentTask);
    }
    task_switch(&Dispatcher);
}

/**
 * Alterna a execução para a tarefa indicada
 * @param *task
 * return ID?
 */
int task_switch (task_t *task){
    if(!task)
        return -1;

    PreviousTask = CurrentTask; 

    CurrentTask = task;
    CurrentTask->status = RODANDO;

    //troca de contexto
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
int task_init (task_t *task, void (*start_routine)(void *),  void *arg){
    
    getcontext(&task->context);
    
    char *stack = malloc(STACKSIZE);
    if(stack){
        task->context.uc_stack.ss_sp = stack ;
        task->context.uc_stack.ss_size = STACKSIZE ;
        task->context.uc_stack.ss_flags = 0 ;
        task->context.uc_link = 0 ;
        task->vg_id = VALGRIND_STACK_REGISTER (task->context.uc_stack.ss_sp, task->context.uc_stack.ss_sp + STACKSIZE);
    }
    else{
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
    //coloca a task no fim da fila de prontas
    //TODO tratar erros de fila
    queue_append((queue_t**)&ReadyQueue,(queue_t*)task);
    
    #ifdef DEBUG
    printf("\nQUEUE SIZE: %d\n",queue_size((queue_t*)ReadyQueue));
    queue_print ("Saida gerada  ", (queue_t*) ReadyQueue, print_elem);
    #endif
    
    //TODO como saber quantos arguemtnos
    makecontext(&task->context,(void *)start_routine, 1, arg);

    if(idCounter > 1)
        userTasks++;
    
    return task->id;
}




/**
 * Termina a tarefa corrente com um status de encerramento
 * @param exit_code (nao sera usado ainda)
 */

int task_id(){
    return CurrentTask->id;
}

void freeQueueTasks(task_t *queue){
    while (queue)
    {
        task_t *elem = queue;
        queue_remove((queue_t**)&queue,(queue_t*)queue);
        if(elem->id != 0){
            if(elem->context.uc_stack.ss_sp){
                free(elem->context.uc_stack.ss_sp);
                // dezfaz o registro da pilha no valgrind
                VALGRIND_STACK_DEREGISTER (elem->vg_id);
            }
        }
    }
    
}