#define MAX_SIZE 100

struct BestResult
{
    int distance;
    int bestPath[MAX_SIZE];
    struct timeval executionTime;
    int iterationsNeeded;
};

void freeMatrix(int **matrix);

void readFile(const char *fileName, int *matrixSize, int ***distances);

int calculateDistance(int *path, int **distances, int size);

void elementRandomSwitch(int *originalPath, int size);

void generateRandomPath(int *path, int size);

void showResults(struct BestResult bestResult, int size);