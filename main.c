#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SIZE 5
#define PROCESSES 5

void read_file(const char *fileName)
{
    FILE *ptr;
    char filePath[100];                             // Define a reasonable size for the file path
    sprintf(filePath, "./tsp_testes/%s", fileName); // Construct the file path

    ptr = fopen(filePath, "r");
    if (ptr == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    int num;
    fscanf(ptr, "%d", &num);
    printf("%d\n", num);
    fclose(ptr);
}

int distances[SIZE][SIZE] = {
    {0, 23, 10, 4, 1},
    {23, 0, 9, 5, 4},
    {10, 9, 0, 8, 2},
    {4, 5, 8, 0, 11},
    {1, 4, 2, 11, 0}};

int calculateDistance(int *path)
{
    int totalDistance = 0;

    // printf("Caminho: %d %d %d %d %d \n", path[0], path[1], path[2], path[3], path[4]);

    for (int i = 0; i < SIZE; i++)
    {
        int src = path[i];
        int dest = path[i + 1];

        if (i == SIZE - 1)
            dest = path[0];

        totalDistance += distances[src][dest];
    }

    return totalDistance;
}

void elementSwitch(int *orginalPath)
{
    int pos1 = rand() % SIZE;
    int pos2;

    do
    {
        pos2 = rand() % SIZE;
    } while (pos2 == pos1);

    int temp;

    temp = orginalPath[pos1];
    orginalPath[pos1] = orginalPath[pos2];
    orginalPath[pos2] = temp;
}

int main()
{
    srand(time(NULL));

    int generatedNumbers[SIZE];
    int count = 0;

    while (count < SIZE)
    {
        int num = rand() % SIZE;
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