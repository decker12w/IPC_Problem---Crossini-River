// Biblitecas Utilizadas
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
int boarded = 0;
int hackers_on_boat = 0;
int serfs_on_boat = 0;
int cont_pessoa = 0;

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

    if (index == 0) // faz o semáforo esperar o valor de um semáforo (hacker)
    {
        sem_wait(&hacker_queue);
        hackers_on_boat++;
    }
    else // faz o semáforo esperar o valor de um semáforo (serf)
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
    return NULL;
}

int main()
{
    // Iniciando as threads e as barreiras
    pthread_t th[ThreadsMAX];
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&pessoasBarco, NULL, boatNumber);
    sem_init(&hacker_queue, 0, 0);
    sem_init(&serf_queue, 0, 0);


    int pessoas[ThreadsMAX]; // possibilidades máximas de pessoas 16
    int opc;
    do
    {
        // Menu de opções
        printf("Escolha entre as opções\n");
        printf("1 - Adicionar hacker\n");
        printf("2 - Adicionar serf\n");
        printf("3 - Executar programa\n");
        scanf("%d", &opc);

        if (cont_pessoa == ThreadsMAX) // caso atingir o número máximo
        {
            printf("Desculpe o número máxima de pessoas foi atingida\n");
            printf("Iremos executar o código\n");
            continue;
        }
        switch (opc)
        {
        case 1: //adicionando hacker
            pessoas[cont_pessoa] = 0;
            cont_pessoa++;
            break;
        case 2: // adicionando serf
            pessoas[cont_pessoa] = 1;
            cont_pessoa++;
            break;
        case 3: // executando o programa
            break;
        default: // opc inválida
            printf("Número inválido\nDigite outro\n");
            scanf("%d", &opc);
            break;
        }
        printf("\n\n\n");
    } while (opc != 3);

    // Executando a rotina nas threads
    for (int i = 0; i < cont_pessoa; i++)
    {
        int *a = malloc(sizeof(int));
        *a = pessoas[i];
        if (pthread_create(&th[i], NULL, &routine, a) != 0)
        {
            perror("pthread_create");
        }
    }   


    // Esperando todas as threads terminarem sua execução    
    for (int i = 0; i < cont_pessoa; i++)
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

// Funções para printar na tela o andamento do programa
void board(int arg)
{
    printf("Entrando no barco %s\n", arg == 0 ? "hackers" : "serfs");
}

void rowboard(int arg)
{
    printf("Remando %s\n", arg == 0 ? "hackers" : "serfs");
};
