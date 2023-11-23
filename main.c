#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/wait.h>

int **distances;
int matrixSize;

// Função para liberar a memória alocada para a matriz
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
void elementSwitch(int *orginalPath)
{
    int pos1 = rand() % matrixSize;
    int pos2;

    do
    {
        pos2 = rand() % matrixSize;
    } while (pos2 == pos1);

    int temp;

    temp = orginalPath[pos1];
    orginalPath[pos1] = orginalPath[pos2];
    orginalPath[pos2] = temp;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("%d", argc);
        printf("ERRO: Número de argumentos inválido!");
        exit(-1);
    }

    // Obtém os argumentos do comnado executado
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

    // Inicializa um caminho aleatório
    srand(time(NULL));

    struct timeval tvi, tvf, tv_res;
    gettimeofday(&tvi, NULL);

    // Memória partilhada
    int size = sizeof(struct BestResult);
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_ANONYMOUS | MAP_SHARED;
    void *shmem = mmap(NULL, size, protection, visibility, 0, 0);

    struct BestResult *bestResult = (struct BestResult *)shmem;

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

    // Executa o algoritmo em cada processo
    for (int i = 0; i < PROCESSES; i++)
    {
        if (fork() == 0)
        {
            // Calcula a distância do caminho inicial
            int total = calculateDistance(generatedNumbers);

            time_t startTime = time(NULL);
            int iterations = 0;
            while (1)
            {
                iterations++;
                elementSwitch(generatedNumbers);
                int tempTotal = calculateDistance(generatedNumbers);

                if (tempTotal < total)
                {
                    gettimeofday(&tvf, NULL);
                    timersub(&tvf, &tvi, &tv_res);

                    total = tempTotal;
                    bestResult->distance = total;
                    bestResult->iterationsNeeded = iterations;
                    bestResult->executionTime = tv_res;
                    memcpy(bestResult->bestPath, generatedNumbers, matrixSize * sizeof(int));
                }

                time_t currentTime = time(NULL);
                if ((currentTime - startTime) >= TIME)
                    exit(0);
            }
        }
    }

    // Aguarda os processos filhos terminarem
    for (int i = 0; i < PROCESSES; i++)
    {
        wait(NULL);
    }

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