#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#include "utilities.h"

#define MAX_SIZE 100

int **distances; // Matriz de distâncias
int matrixSize;  // Tamanho da matriz

int PROCESSES = 0;

int total; // Melhor caminhos

void *shmem;

struct BestResult
{
    int distance;
    int bestPath[MAX_SIZE];
    struct timeval executionTime;
    int iterationsNeeded;
};

struct BestResult *bestResult;

int *childPIDs; // PIDs dos processos filhos

// Informa os filhos para atualizarem o melhor caminho
void handleBestPath(int signal)
{
    for (int i = 0; i < PROCESSES; i++)
    {
        kill(childPIDs[i], SIGUSR2);
    }
}

// Mata os filhos quando o tempo termina
void handleTimer(int signal)
{
    for (int i = 0; i < PROCESSES; i++)
    {
        kill(childPIDs[i], SIGKILL);
    }
}

// Atualiza o melhor caminho
void refreshTotal(int signal)
{
    total = bestResult->distance;
}

int main(int argc, char *argv[])
{
    /* LEITURA DE AEGUMENTOS */
    if (argc != 4)
    {
        printf("ERRO: Número de argumentos inválido!");
        exit(-1);
    }

    // Obtém os argumentos do comando executado
    const char *TEST_FILE = argv[1];
    PROCESSES = atoi(argv[2]);
    const int TIME = atoi(argv[3]);

    /* DEFINIÇÂO DE SINAIS */
    signal(SIGALRM, handleTimer);

    signal(SIGUSR1, handleBestPath);

    signal(SIGUSR2, refreshTotal);

    readFile(TEST_FILE, &matrixSize, &distances);

    // Memória partilhada
    int size = sizeof(struct BestResult);
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_ANONYMOUS | MAP_SHARED;
    shmem = mmap(NULL, size, protection, visibility, 0, 0);

    bestResult = (struct BestResult *)shmem;

    // Semáforo de acesso à memória partilhada
    sem_unlink("memoryAccess");
    sem_t *memoryAccess = sem_open("memoryAccess", O_CREAT, 0644, 1);

    // Array para armazenar os ID's dos processos filhos
    childPIDs = (int *)malloc(PROCESSES * sizeof(int));

    // Inicia a contagem do tempo para terminar
    alarm(TIME);

    // Executa o algoritmo em cada processo
    for (int i = 0; i < PROCESSES; i++)
    {
        childPIDs[i] = fork();
        if (childPIDs[i] == 0)
        {
            /* INICIALIZAÇÃO DE CAMINHO ALEATÓRIO */
            srand(getppid());

            int generatedNumbers[matrixSize];
            generateRandomPath(generatedNumbers, matrixSize);

            struct timeval tvi, tvf, tv_res;
            gettimeofday(&tvi, NULL);

            // Calcula a distância do caminho inicial
            total = calculateDistance(generatedNumbers, distances, matrixSize);

            int iterations = 0;
            while (1)
            {
                iterations++;
                elementRandomSwitch(generatedNumbers, matrixSize);
                int tempTotal = calculateDistance(generatedNumbers, distances, matrixSize);

                // Melhor caminho encontrado
                if (tempTotal < total)
                {
                    // Entra no semáforo
                    sem_wait(memoryAccess);

                    // Obtém o tempo de execução
                    gettimeofday(&tvf, NULL);
                    timersub(&tvf, &tvi, &tv_res);

                    // Atualiza os valores na memória partilhada
                    total = tempTotal;
                    bestResult->distance = total;
                    bestResult->iterationsNeeded = iterations;
                    bestResult->executionTime = tv_res;

                    for (int i = 0; i < matrixSize; i++)
                    {
                        bestResult->bestPath[i] = generatedNumbers[i];
                    }

                    // Envia o sinal ao processo pai
                    kill(getppid(), SIGUSR1);

                    // Sai do semáforo
                    sem_post(memoryAccess);
                }
            }

            exit(0);
        }
    }

    // Espera que os filhos terminem
    for (int i = 0; i < sizeof(childPIDs) / sizeof(childPIDs[0]); i++)
    {
        wait(NULL);
    }

    /* RESULTADOS */
    printf("Best Distance: %d\n", bestResult->distance);
    printf("Best Path: ");
    for (int i = 0; i < matrixSize; i++)
    {
        printf("%d ", bestResult->bestPath[i]);
    }
    printf("\n");
    printf("Time: %0ld.%03ld s\n", (long)bestResult->executionTime.tv_sec, (long)bestResult->executionTime.tv_usec / 1000);
    printf("Iterations: %d\n", bestResult->iterationsNeeded);

    // Liberta os espaços de memória alocados
    sem_close(memoryAccess);
    freeMatrix(distances, matrixSize);
    free(childPIDs);

    return 0;
}