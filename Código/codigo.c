// Biblitecas Utilizadas
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

// Definindo valores fixos
#define ThreadsNumber 16
#define boatNumber 4

void board(int arg);
void rowboard(int arg);

// Definindo variáveis globais
int hackers = 0;
int serfs = 0;
int boarded = 0;
int hackers_on_boat = 0;
int serfs_on_boat = 0;

// Variáveis globais de barreias, threads e mutex
pthread_barrier_t pessoasBarco;
pthread_mutex_t mutex;
sem_t hacker_queue;
sem_t serf_queue;

void *routine(void *args)
{
    int index = *(int *)args;
    bool isCaptain = false;

    pthread_mutex_lock(&mutex);
    if (index == 0)
    {
        hackers++;
        if (hackers == 4)
        {
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            hackers -= 4;
            isCaptain = true;
        }
        else if (hackers == 2 && serfs >= 2)
        {
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            hackers -= 2;
            serfs -= 2;
            isCaptain = true;
        }
        else
        {
            pthread_mutex_unlock(&mutex);
        }
    }
    else
    {
        serfs++;
        if (serfs == 4)
        {
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            serfs -= 4;
            isCaptain = true;
        }
        else if (hackers >= 2 && serfs >= 2)
        {
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            hackers -= 2;
            serfs -= 2;
            isCaptain = true;
        }
        else
        {
            pthread_mutex_unlock(&mutex);
        }
    }

    if (index == 0)
    {
        sem_wait(&hacker_queue);
        hackers_on_boat++;
    }
    else
    {
        sem_wait(&serf_queue);
        serfs_on_boat++;
    }

    board(index);

    pthread_barrier_wait(&pessoasBarco);

    if (isCaptain)
    {
        printf("Hackers no barco: %d\n", hackers_on_boat);
        printf("Serfs no barco: %d\n", serfs_on_boat);
        hackers_on_boat = 0;
        serfs_on_boat = 0;
        rowboard(index);
        pthread_mutex_unlock(&mutex);
    }

    free(args);
}

int main()
{
    // Iniciando as threads e as barreiras
    pthread_t th[ThreadsNumber];
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&pessoasBarco, NULL, boatNumber);
    sem_init(&hacker_queue, 0, 0);
    sem_init(&serf_queue, 0, 0);

    // Ordem que os hackers e serfs irão entrar, sendo hackers 0 e serfs 1
    int pessoas[ThreadsNumber] = {1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0};

    // Executando a rotina nas threads
    for (int i = 0; i < ThreadsNumber; i++)
    {
        int *a = malloc(sizeof(int));
        *a = pessoas[i];

        if (pthread_create(&th[i], NULL, &routine, a) != 0)
        {
            perror("pthread_create");
        }
    }

    for (int i = 0; i < ThreadsNumber; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            perror("pthread_join");
        }
    }

    // Destruindo os arquivos resíduais e terminando o programa
    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&pessoasBarco);
    sem_destroy(&hacker_queue);
    sem_destroy(&serf_queue);
    return 0;
}

// Funções para printar na tela a execução do programa
void board(int arg)
{
    printf("Entrando no barco %s\n", arg == 0 ? "hackers" : "serfs");
}

void rowboard(int arg)
{
    printf("Remando %s\n", arg == 0 ? "hackers" : "serfs");
};
