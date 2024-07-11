#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

// Definindo valores fixos
#define ThreadsMAX 20
#define boatNumber 4

void board(int arg);
void rowboard(int arg);

// Definindo variáveis globais
int hackers = 0;
int serfs = 0;
int hackers_on_boat = 0;
int serfs_on_boat = 0;
int cont_pessoa = 0;

// Variáveis globais de barreiras, threads e mutex
pthread_barrier_t pessoasBarco;
pthread_mutex_t mutex;
pthread_mutex_t boat_mutex;
sem_t hacker_queue;
sem_t serf_queue;

void *routine(void *args)
{
    // José
    sleep(rand() % 5); // sleep para simular a chegada de pessoas (hackers e serfs)
    int index = *(int *)args;
    bool isCaptain = false;

    // Taichi
    pthread_mutex_lock(&mutex);
    if (index == 0) // caso hackers entrarem no barco
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
    { // caso serfs entrarem no barco
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

    // José
    if (index == 0) // faz o semáforo esperar o valor de um semáforo (hacker)
    {
        sem_wait(&hacker_queue);
        pthread_mutex_lock(&boat_mutex);
        hackers_on_boat++;
        pthread_mutex_unlock(&boat_mutex);
    }
    else // faz o semáforo esperar o valor de um semáforo (serf)
    {
        sem_wait(&serf_queue);
        pthread_mutex_lock(&boat_mutex);
        serfs_on_boat++;
        pthread_mutex_unlock(&boat_mutex);
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
    return NULL;
}

int main()
{
    // José
    pthread_t th[ThreadsMAX];
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&boat_mutex, NULL);
    pthread_barrier_init(&pessoasBarco, NULL, boatNumber);
    sem_init(&hacker_queue, 0, 0);
    sem_init(&serf_queue, 0, 0);

    int pessoas[ThreadsMAX];
    int num_hackers, num_serfs;

    printf("Digite a quantidade de hackers: ");
    scanf("%d", &num_hackers);
    printf("Digite a quantidade de serfs: ");
    scanf("%d", &num_serfs);

    if (num_hackers + num_serfs > ThreadsMAX)
    {
        printf("Erro: A soma de hackers e serfs excede o limite máximo de %d.\n", ThreadsMAX);
        return 1;
    }

    for (int i = 0; i < num_hackers; i++, cont_pessoa++)
    {
        pessoas[cont_pessoa] = 0; // 0 representa hacker
    }

    for (int i = 0; i < num_serfs; i++, cont_pessoa++)
    {
        pessoas[cont_pessoa] = 1; // 1 representa serf
    }

    for (int i = 0; i < cont_pessoa; i++)
    {
        int *a = malloc(sizeof(int));
        *a = pessoas[i];
        if (pthread_create(&th[i], NULL, &routine, a) != 0)
        {
            perror("pthread_create");
        }
    }

    for (int i = 0; i < cont_pessoa; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            perror("pthread_join");
        }
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&boat_mutex);
    pthread_barrier_destroy(&pessoasBarco);
    sem_destroy(&hacker_queue);
    sem_destroy(&serf_queue);
    return 0;
}

// Funções para printar na tela o andamento do programa
void board(int arg)
{
    printf("Entrando no barco %s\n", arg == 0 ? "hackers" : "serfs");
}

void rowboard(int arg)
{
    printf("Remando %s\n", arg == 0 ? "hackers" : "serfs");
}
