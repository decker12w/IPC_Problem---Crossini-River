#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

#define ThreadsNumber 8
#define boatNumber 4

void board();
void rowboard();

int hackers = 0;
int serfs = 0;
int boarded = 0;

pthread_barrier_t pessoasBarco;
pthread_mutex_t mutex;
sem_t hacker_sem;
sem_t serf_sem;

void *routine(void *args)
{
    int index = *(int *)args;
    if (index == 0)
    {
        sem_wait(&hacker_sem);
        pthread_mutex_lock(&mutex);
        hackers++;
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        sem_wait(&serf_sem);
        pthread_mutex_lock(&mutex);
        serfs++;
        pthread_mutex_unlock(&mutex);
    }

    sleep(1);
    board(index);
    sem_post(&hacker_sem);
    sem_post(&serf_sem);

    pthread_mutex_lock(&mutex);
    boarded++;
    bool isCaptain = (boarded % boatNumber == 0);
    pthread_mutex_unlock(&mutex);

    pthread_barrier_wait(&pessoasBarco);

    if (isCaptain)
    {
        rowboard(index);
    }

    free(args);
}

int main()
{

    pthread_t th[ThreadsNumber];
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&pessoasBarco, NULL, boatNumber);
    sem_init(&hacker_sem, 0, 2);
    sem_init(&serf_sem, 0, 2);

    int pessoas[ThreadsNumber] = {0, 1, 0, 1, 0, 1, 0, 1};

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

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&pessoasBarco);
    sem_destroy(&hacker_sem);
    sem_destroy(&serf_sem);
    printf("Hackers: %d\n", hackers);
    printf("Serfs: %d\n", serfs);

    return 0;
}

void board(int arg)
{
    printf("Entrando no barco %s\n", arg == 0 ? "hackers" : "serfs");
}

void rowboard(int arg)
{
    printf("Remando %s\n", arg == 0 ? "hackers" : "serfs");
};
