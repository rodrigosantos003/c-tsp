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

int **distances;
int matrixSize;

// Liberta a memória alocada à matriz
void freeMatrix()
{
    for (int i = 0; i < matrixSize; ++i)
    {
        free(distances[i]);
    }
    free(distances);
}

// Lê o ficheiro e inicializa a matriz
void read_file(const char *fileName)
{
    FILE *file;
    char filePath[100];
    sprintf(filePath, "./tsp_testes/%s", fileName);

    file = fopen(filePath, "r");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    fscanf(file, "%d", &matrixSize);

    distances = (int **)malloc(matrixSize * sizeof(int *));
    for (int i = 0; i < matrixSize; ++i)
    {
        distances[i] = (int *)malloc(matrixSize * sizeof(int));
        for (int j = 0; j < matrixSize; ++j)
        {
            fscanf(file, "%d", &distances[i][j]);
        }
    }

    fclose(file);
}

// Calcula a distância de um dado caminho
int calculateDistance(int *path)
{
    int totalDistance = 0;

    for (int i = 0; i < matrixSize; i++)
    {
        int src = path[i];
        int dest = path[i + 1];

        if (i == matrixSize - 1)
            dest = path[0];

        totalDistance += distances[src][dest];
    }

    return totalDistance;
}

// Troca os pontos do caminho
void elementSwitch(int *originalPath)
{
    int pos1 = rand() % matrixSize;
    int pos2;

    do
    {
        pos2 = rand() % matrixSize;
    } while (pos2 == pos1);

    int temp;

    temp = originalPath[pos1];
    originalPath[pos1] = originalPath[pos2];
    originalPath[pos2] = temp;
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
    const int PROCESSES = atoi(argv[2]);
    const int TIME = atoi(argv[3]);

    read_file(TEST_FILE);

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

    /* INICIALIZAÇÃO DE CAMINHO ALEATÓRIO */
    srand(time(NULL));

    int generatedNumbers[matrixSize];
    int count = 0;

    while (count < matrixSize)
    {
        int num = rand() % matrixSize;
        int repeated = 0;

        // Verifica se o número já foi gerado antes
        for (int i = 0; i < count; i++)
        {
            if (num == generatedNumbers[i])
            {
                repeated = 1;
                break;
            }
        }

        if (!repeated)
        {
            generatedNumbers[count] = num;
            count++;
        }
    }

    /* AJ-PE NOS PROCESSOS*/
    // Inicialização das estruturas de tempo
    struct timeval tvi, tvf, tv_res;
    gettimeofday(&tvi, NULL);

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
            // Calcula a distância do caminho inicial
            int total = calculateDistance(generatedNumbers);

            int iterations = 0;
            while (1)
            {
                iterations++;
                elementSwitch(generatedNumbers);
                int tempTotal = calculateDistance(generatedNumbers);

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
    printf("Time: %0ld.%03ld ms\n", (long)bestResult->executionTime.tv_sec, (long)bestResult->executionTime.tv_usec / 1000);
    printf("Iterations: %d\n", bestResult->iterationsNeeded);
    printf("\n");

    freeMatrix();

    return 0;
}