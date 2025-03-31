#include "ppos_data.h"
#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

#define STACKSIZE 64*1024

task_t MainTask; ucontext_t CurrentContext;


/**
 * Inicializa o sistema operaconal; deve ser chamado no inicio do main()
 */
void ppos_init(){
    
    //desativa o buffer do printf
    setvbuf (stdout, 0, _IONBF, 0) ;
    
    //inicializa MainTask
    MainTask.prev = NULL;
    MainTask.next = NULL;
    MainTask.id = 0; //main context sempre é 0;
    getcontext(&MainTask.context); //contexto atual em MainContext
    char *stack = malloc(STACKSIZE);
    if(stack){
        MainTask.context.uc_stack.ss_sp = stack ;
        MainTask.context.uc_stack.ss_size = STACKSIZE ;
        MainTask.context.uc_stack.ss_flags = 0 ;
        MainTask.context.uc_link = 0 ;
    }
    else{
        perror ("Erro na criação da pilha: ") ;
        exit (1) ;
    }
    //TODO como fazer isso
    //makecontext (&MainTask.context, (void*)(*main), 0,) ;
    MainTask.status = 1; //0 pronta, 1 rodando -1 suspensa?

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
    task->id = aux->id + 1;  //TODO função que itera tarefas e ve qual ID está disponivel
    
    //TODO como saber quantos
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
    //if(!&task->context)
    //    return -1;
    getcontext(&CurrentContext);

    swapcontext(&CurrentContext,&task->context);

    return 0;
    
}


/**
 * Termina a tarefa corrente com um status de encerramento
 * @param exit_code (nao sera usado ainda)
 */
void task_exit (int exit_code){

    task_switch(&MainTask);
    //TODO free da pilha
}