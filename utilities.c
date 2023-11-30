#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utilities.h"

#define MAX_SIZE 100

// Liberta a memória alocada à matriz
void freeMatrix(int **matrix)
{
    for (int i = 0; i < sizeof(matrix) / sizeof(matrix[0]); ++i)
    {
        free(matrix[i]);
    }
    free(matrix);
}

// Lê o ficheiro e inicializa a matriz
void readFile(const char *fileName, int *matrixSize, int ***distances)
{
    FILE *file;
    char filePath[MAX_SIZE];
    sprintf(filePath, "./tsp_testes/%s", fileName);

    file = fopen(filePath, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%d", matrixSize);

    *distances = (int **)malloc(*matrixSize * sizeof(int *));
    for (int i = 0; i < *matrixSize; ++i)
    {
        (*distances)[i] = (int *)malloc(*matrixSize * sizeof(int));
        for (int j = 0; j < *matrixSize; ++j)
        {
            fscanf(file, "%d", &((*distances)[i][j]));
        }
    }

    fclose(file);
}

// Calcula a distância de um dado caminho
int calculateDistance(int *path, int **distances, int size)
{
    int totalDistance = 0;

    for (int i = 0; i < size; i++)
    {
        int src = path[i];
        int dest = path[(i + 1) % size];
        totalDistance += distances[src][dest];
    }

    return totalDistance;
}

// Troca os pontos do caminho
void elementRandomSwitch(int *originalPath, int size)
{
    int pos1 = rand() % size;
    int pos2;

    do
    {
        pos2 = rand() % size;
    } while (pos2 == pos1);

    int temp = originalPath[pos1];
    originalPath[pos1] = originalPath[pos2];
    originalPath[pos2] = temp;
}

// Gera um caminho aleatório
void generateRandomPath(int *path, int size)
{
    int count = 0;

    while (count < size)
    {
        int num = rand() % size;
        int repeated = 0;

        for (int i = 0; i < count; i++)
        {
            if (num == path[i])
            {
                repeated = 1;
                break;
            }
        }

        if (!repeated)
        {
            path[count] = num;
            count++;
        }
    }
}

// Apresenta os resultados
void showResults(struct SharedMemory sharedMemory, int size)
{
    printf("Best Distance: %d\n", sharedMemory.distance);
    printf("Best Path: ");
    for (int i = 0; i < size; i++)
    {
        printf("%d ", sharedMemory.bestPath[i]);
    }
    printf("\n");
    if (sharedMemory.executionTime.tv_usec < 1000)
        printf("Time: 0.%03ld ms\n", (long)sharedMemory.executionTime.tv_usec);
    else
        printf("Time: %0ld.%03ld s\n", (long)sharedMemory.executionTime.tv_sec, (long)sharedMemory.executionTime.tv_usec / 1000);
    printf("Iterations: %d\n", sharedMemory.iterationsNeeded);
    printf("------------------------\n");
}