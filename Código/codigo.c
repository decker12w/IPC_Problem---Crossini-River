#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

// Definindo valores fixos
#define ThreadsMAX 50000
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
pthread_mutex_t mutex;
pthread_mutex_t boat_mutex;
pthread_barrier_t barrier;
sem_t hacker_queue;
sem_t serf_queue;

void *routine(void *args)
{
    // sleep para simular a chegada de pessoas (hackers e serfs)
    sleep(rand() % 1);
    // Define se é hacker (0) ou serf (1)
    int index = *(int *)args;
    bool isCaptain = false;

    // Mutex para proteger a seção crítica onde hackers e serfs são contados
    pthread_mutex_lock(&mutex);
    if (index == 0) // Caso hackers entrarem no barco
    {
        hackers++;
        if (hackers == 4) // Se há 4 hackers, eles formam um grupo e entram no barco
        {
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            hackers -= 4;
            isCaptain = true; // um hacker é designado como capitão
        }
        else if (hackers == 2 && serfs >= 2) // Se há 2 hackers ou 2 serfs, eles formam um grupo e podem entrar no barco
        {
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            hackers -= 2;
            serfs -= 2;
            isCaptain = true; // Um hacker ou serf é designado como capitão
        }
        else
        {
            pthread_mutex_unlock(&mutex); // Libera o mutex se não formou o grupo
        }
    }
    else
    { // caso serfs entrarem no barco
        serfs++;
        if (serfs == 4) // Se há 4 serfs, formam um grupo e entram no barco
        {
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            serfs -= 4;
            isCaptain = true; // Um serf é capitão
        }
        else if (hackers >= 2 && serfs >= 2) // Se há 2 hackers ou 2 serfs, eles formam um grupo e podem entrar no barco
        {
            sem_post(&hacker_queue);
            sem_post(&hacker_queue);
            sem_post(&serf_queue);
            sem_post(&serf_queue);
            hackers -= 2;
            serfs -= 2;
            isCaptain = true; // Ou um hacker ou um serf é capitão
        }
        else
        {
            pthread_mutex_unlock(&mutex); // Libera o mutex se não formou o grupo
        }
    }

    // Hacker ou serf espera até que seu semáforo seja liberado
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

    // Função board quando entra no barco
    board(index);

    // Implementação da barreira usando pthread_barrier_wait
    pthread_barrier_wait(&barrier);

    if (isCaptain)
    {
        printf("Hackers no barco: %d\n", hackers_on_boat);
        printf("Serfs no barco: %d\n", serfs_on_boat);
        hackers_on_boat = 0;
        serfs_on_boat = 0;
        rowboard(index);              // Capitão chama a função para remar
        pthread_mutex_unlock(&mutex); // Capitão libera o mutex para permitir novos grupos
    }

    free(args);
    return NULL;
}

int main()
{
    // Variáveis
    pthread_t th[ThreadsMAX];
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&boat_mutex, NULL);
    pthread_barrier_init(&barrier, NULL, boatNumber);
    sem_init(&hacker_queue, 0, 0);
    sem_init(&serf_queue, 0, 0);

    int pessoas[ThreadsMAX];
    int num_hackers, num_serfs;

    srand(getpid());

    // Escolha de quantidade de hacker e serfs
    printf("Digite a quantidade de hackers: ");
    scanf("%d", &num_hackers);
    printf("Digite a quantidade de serfs: ");
    scanf("%d", &num_serfs);

    // Caso estoure o limite máximo
    if (num_hackers + num_serfs > ThreadsMAX)
    {
        printf("Erro: A soma de hackers e serfs excede o limite máximo de %d.\n", ThreadsMAX);
        return 1;
    }

    // Inicia o array com hackers ou serfs
    for (int i = 0; i < num_hackers; i++, cont_pessoa++)
    {
        pessoas[cont_pessoa] = 0; // 0 representa hacker
    }

    for (int i = 0; i < num_serfs; i++, cont_pessoa++)
    {
        pessoas[cont_pessoa] = 1; // 1 representa serf
    }

    // Criação de threads
    for (int i = 0; i < cont_pessoa; i++)
    {
        int *a = malloc(sizeof(int));
        *a = pessoas[i];
        if (pthread_create(&th[i], NULL, &routine, a) != 0)
        {
            perror("pthread_create");
        }
    }

    // Espera todas as threads terminarem
    for (int i = 0; i < cont_pessoa; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            perror("pthread_join");
        }
    }

    // Destrói mutex, barreira e semáforos
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&boat_mutex);
    pthread_barrier_destroy(&barrier);
    sem_destroy(&hacker_queue);
    sem_destroy(&serf_queue);
    pthread_barrier_init(&barrier, NULL, boatNumber);
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
