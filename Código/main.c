#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <bits/pthreadtypes.h>

#define SEM_OXYGEN "/oxygen"
#define SEM_HIDROGEN "/hidrogen"

// Definindo a barreira
pthread_barrier_t barrier;

void *routine(void *args);

int main(void)
{
    //Criação das barreiras
    pthread_barrier_init(&barrier, NULL, 3);

    //Criação das threads
    pthread_t hidrogenio[2];
    pthread_t oxigenio;

    for(int i = 0; i<2; i++){
        if(pthread_create(&hidrogenio[i], NULL, routine, NULL)!=0){
            perror("Falha na criação da thread\n");
        }
    }
    if(pthread_create(&oxigenio, NULL, routine, NULL) !=0){
        perror("Falha na criação da thread");
    }

    for(int i = 0; i<2; i++){
        if(pthread_join(hidrogenio[i], NULL)!=0){
            perror("Falha ao entrar na thread\n");
        }
    }

    if(pthread_join(oxigenio, NULL)!=0){
        perror("Falha ao entrar na thread\n");
    }

    // Criação dos semáforos
    sem_unlink(SEM_OXYGEN);
    sem_unlink(SEM_HIDROGEN);
    sem_t *oxy_queue = sem_open(SEM_OXYGEN, IPC_CREAT, 0660,0);
    sem_t *hdro_queue = sem_open(SEM_HIDROGEN, IPC_CREAT, 0660,0);
    if(oxy_queue == SEM_FAILED){
        perror("Falha na criação do semáforo oxy\n");
        exit(EXIT_FAILURE);
    }
    if(hdro_queue == SEM_FAILED){
        perror("Falha na criação do semáforo hdro\n");
        exit(EXIT_FAILURE);
    }


    //Terminando programa
    pthread_barrier_destroy(&barrier);
    return 0;
}

void* routine(void* args)
{
    printf("Esperando a barreira\n");
    pthread_barrier_wait(&barrier);
    printf("Nós passamos a barreira\n");
    return NULL;
}