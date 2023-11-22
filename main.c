#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int **distances;
int matrixSize;

// Função para liberar a memória alocada para a matriz
void freeMatrix() {
    for (int i = 0; i < matrixSize; ++i) {
        free(distances[i]);
    }
    free(distances);
}

// Lê o ficheiro e inicializa a matriz
void read_file(const char *fileName) {
    FILE *file;
    char filePath[100];
    sprintf(filePath, "./tsp_testes/%s", fileName);

    file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    fscanf(file, "%d", &matrixSize);

    distances = (int **)malloc(matrixSize * sizeof(int *));
    for (int i = 0; i < matrixSize; ++i) {
        distances[i] = (int *)malloc(matrixSize * sizeof(int));
        for (int j = 0; j < matrixSize; ++j) {
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
    // Obtém os argumentos do comnado executado
    const char *TEST_FILE = argv[1];
    const int PROCESSES = atoi(argv[2]);
    const int TIME = atoi(argv[3]);

    read_file(TEST_FILE);

    // Inicializa um caminho aleatório
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

    // Executa o algoritmo em cada processo
    for (int i = 0; i < PROCESSES; i++)
    {
        if (fork() == 0)
        {
            //Calcula a distância do caminho inicial
            int total = calculateDistance(generatedNumbers);
            printf("Total inicial: %d\n", total);
        
            for (int i = 0; i < 10; i++)
            {
                elementSwitch(generatedNumbers);
                int tempTotal = calculateDistance(generatedNumbers);
                printf("Total nº %d: %d\n", i, tempTotal);

                if (tempTotal < total)
                    total = tempTotal;
            }

            printf("Maior total: %d\n", total);

            exit(0);
        }
    }

    freeMatrix();

    return 0;
}