void freeMatrix(int** matrix, int size);

void readFile(const char* fileName, int* matrixSize, int*** distances);

int calculateDistance(int* path, int** distances, int size);

void elementRandomSwitch(int* originalPath, int size);

void generateRandomPath(int* path, int size);