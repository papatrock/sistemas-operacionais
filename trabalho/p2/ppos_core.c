/*
    Patrick Oliveira Lemes
    GRR20211777
*/

#include "ppos_data.h"
#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

#define STACKSIZE 64*1024

task_t MainTask, *CurrentTask, *PreviousTask;


/**
 * Inicializa o sistema operaconal; deve ser chamado no inicio do main()
 */
void ppos_init(){
    
    //desativa o buffer do printf
    setvbuf (stdout, 0, _IONBF, 0) ;
    
    //inicializa MainTask
    MainTask.id = 0; //main context sempre é 0;
    MainTask.status = 1; //rodando
    
    CurrentTask = &MainTask;

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
    }
    else{
        perror("Erro na criação da pilha: ");
        return -1;
    }
    
    task_t *aux = &MainTask;
    while(aux->next != NULL)
        aux = aux->next;

    aux->next = task;
    task->prev = aux;
    task->next = NULL;
    task->id = aux->id + 1;
    task->status = 0; // pronta
    
    //TODO como saber quantos arguemtnos
    makecontext(&task->context,(void *)start_routine, 1, arg);
    task->status = 0;
    

    return task->id;
}


/**
 * Alterna a execução para a tarefa indicada
 * @param *task
 * return ID?
 */
int task_switch (task_t *task){
    if(!task)
        return -1;
    
    task->prev = CurrentTask;
    CurrentTask = task;
    task->status = 1;
    task->prev->status = 0;
    if (task->prev && task->prev != task) {
        swapcontext(&task->prev->context, &task->context);
    } else {
        setcontext(&task->context);
    }
    
    
    return 0;
    
}


/**
 * Termina a tarefa corrente com um status de encerramento
 * @param exit_code (nao sera usado ainda)
 */
void task_exit (int exit_code){

     task_switch(&MainTask);

     perror("Erro no task_exit");
     exit(1);
}

int task_id(){
    return CurrentTask->id;
}