#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MATRIX_SIZE 20
#define NUM_THREADS 10

int MAX;

int matA[MATRIX_SIZE][MATRIX_SIZE];
int matB[MATRIX_SIZE][MATRIX_SIZE];

int matSumResult[MATRIX_SIZE][MATRIX_SIZE];
int matDiffResult[MATRIX_SIZE][MATRIX_SIZE];
int matProductResult[MATRIX_SIZE][MATRIX_SIZE];

typedef struct {
    int startRow;
    int endRow;
} ThreadData;

void fillMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

void printMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* Compute matrix addition for assigned rows. */
void* computeSum(void* args) {
    ThreadData* data = (ThreadData*)args;

    for (int i = data->startRow; i < data->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            matSumResult[i][j] = matA[i][j] + matB[i][j];
        }
    }

    return NULL;
}

/* Compute matrix subtraction for assigned rows. */
void* computeDiff(void* args) {
    ThreadData* data = (ThreadData*)args;

    for (int i = data->startRow; i < data->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
        }
    }

    return NULL;
}

/* Compute matrix multiplication for assigned rows. */
void* computeProduct(void* args) {
    ThreadData* data = (ThreadData*)args;

    for (int i = data->startRow; i < data->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            int sum = 0;

            for (int k = 0; k < MAX; k++) {
                sum += matA[i][k] * matB[k][j];
            }

            matProductResult[i][j] = sum;
        }
    }

    return NULL;
}

/* Each worker computes all three operations for its assigned rows. */
void* computeAll(void* args) {
    computeSum(args);
    computeDiff(args);
    computeProduct(args);

    return NULL;
}

int main(int argc, char* argv[]) {
    srand(time(0));

    /* Get the matrix size from the command line. */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <matrix_size>\n", argv[0]);
        return 1;
    }

    char* endptr;
    long size = strtol(argv[1], &endptr, 10);

    if (*argv[1] == '\0' || *endptr != '\0' ||
        size < 1 || size > MATRIX_SIZE) {
        fprintf(stderr, "Matrix size must be between 1 and 20.\n");
        return 1;
    }

    MAX = (int)size;

    /* Fill the input matrices. */
    fillMatrix(matA);
    fillMatrix(matB);

    /* Print the initial matrices. */
    printf("Matrix A:\n");
    printMatrix(matA);

    printf("Matrix B:\n");
    printMatrix(matB);

    /* Create exactly 10 worker threads. */
    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    int rowsPerThread = (MAX + NUM_THREADS - 1) / NUM_THREADS;
    int threadsCreated = 0;

    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].startRow = i * rowsPerThread;
        threadData[i].endRow = threadData[i].startRow + rowsPerThread;

        if (threadData[i].endRow > MAX) {
            threadData[i].endRow = MAX;
        }

        if (threadData[i].startRow >= MAX) {
            break;
        }

        int result = pthread_create(
            &threads[i],
            NULL,
            computeAll,
            &threadData[i]
        );

        if (result != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            return 1;
        }

        threadsCreated++;
    }

    /* Wait for all worker threads to finish. */
    for (int i = 0; i < threadsCreated; i++) {
        pthread_join(threads[i], NULL);
    }

    /* Print the results. */
    printf("Results:\n");

    printf("Sum:\n");
    printMatrix(matSumResult);

    printf("Difference:\n");
    printMatrix(matDiffResult);

    printf("Product:\n");
    printMatrix(matProductResult);

    return 0;
}
