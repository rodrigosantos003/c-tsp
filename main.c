#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int matrixSize;

int distances[5][5] = {
    {0, 23, 10, 4, 1},
    {23, 0, 9, 5, 4},
    {10, 9, 0, 8, 2},
    {4, 5, 8, 0, 11},
    {1, 4, 2, 11, 0}};

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

    for (int i = 0; i < matrixSize; i++)
    {
        for (int j = 0; j < matrixSize; j++)
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

    // printf("Caminho: %d %d %d %d %d \n", path[0], path[1], path[2], path[3], path[4]);

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
    const int TEST_FILE = argv[1];
    const int PROCESSES = argv[2];
    const int TIME = argv[3];

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
        }
    }

    return 0;
}