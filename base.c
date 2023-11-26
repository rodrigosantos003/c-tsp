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

int **distances;
int matrixSize;

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
    const int PROCESSES = atoi(argv[2]);
    const int TIME = atoi(argv[3]);

    readFile(TEST_FILE, &matrixSize, &distances);

    struct BestResult
    {
        int distance;
        int bestPath[matrixSize];
        struct timeval executionTime;
        int iterationsNeeded;
    };

    // Memória partilhada
    int size = sizeof(struct BestResult);
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_ANONYMOUS | MAP_SHARED;
    void *shmem = mmap(NULL, size, protection, visibility, 0, 0);

    struct BestResult *bestResult = (struct BestResult *)shmem;

    /* AJ-PE NOS PROCESSOS*/
    // Inicialização das estruturas de tempo
    
    // Semáforo de acesso à memória partilhada
    sem_unlink("memoryAccess");
    sem_t *memoryAccess = sem_open("memoryAccess", O_CREAT, 0644, 1);

    // Array para armazenar os ID's dos processos filhos
    int childPIDs[PROCESSES];

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
            int total = calculateDistance(generatedNumbers, distances, matrixSize);

            int iterations = 0;
            while (1)
            {
                iterations++;
                elementRandomSwitch(generatedNumbers, matrixSize);
                int tempTotal = calculateDistance(generatedNumbers, distances, matrixSize);

                // Melhor caminho encontrado
                if (tempTotal < total)
                {
                    sem_wait(memoryAccess);

                    gettimeofday(&tvf, NULL);
                    timersub(&tvf, &tvi, &tv_res);

                    total = tempTotal;
                    bestResult->distance = total;
                    bestResult->iterationsNeeded = iterations;
                    bestResult->executionTime = tv_res;
                    memcpy(bestResult->bestPath, generatedNumbers, matrixSize * sizeof(int));

                    sem_post(memoryAccess);
                }
            }
        }
    }

    // Mata os filhos quando o tempo acaba
    sleep(TIME);
    for (int i = 0; i < sizeof(childPIDs) / sizeof(childPIDs[0]); i++)
    {
        kill(childPIDs[i], SIGKILL);
    }

    /* RESULTADOS */
    printf("Best Distance: %d\n", bestResult->distance);
    printf("Best Path: ");
    for (int i = 0; i < matrixSize; i++)
    {
        printf("%d ", bestResult->bestPath[i]);
    }
    printf("\n");
    printf("Time: %0ld.%03ld s\n", (long)bestResult->executionTime.tv_sec, (long)bestResult->executionTime.tv_usec);
    printf("Iterations: %d\n", bestResult->iterationsNeeded);

    freeMatrix(distances, matrixSize);
    sem_close(memoryAccess);
    return 0;
}