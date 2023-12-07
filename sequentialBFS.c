#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define NUM_NODES 1004

// Function for sequential graph traversal
void sequentialGraphTraversal(const int *adjMatrix, const int *vector, int *result, int targetNode) {
    for (int i = 0; i < NUM_NODES; ++i) {
        result[i] = 0;
        for (int j = 0; j < NUM_NODES; ++j) {
            result[i] += adjMatrix[i * NUM_NODES + j] * vector[j];
        }
        // Set the target node's count to 0 to avoid self-loop contributions
        if (i == targetNode) {
            result[i] = 0;
        }
    }
}

int main() {
    // Host matrices and vectors
    int *h_adjMatrix = (int *)malloc(NUM_NODES * NUM_NODES * sizeof(int));
    int *h_vector = (int *)malloc(NUM_NODES * sizeof(int));
    int *h_result = (int *)malloc(NUM_NODES * sizeof(int));

    // Initialize the adjacency matrix and vector
    for (int i = 0; i < NUM_NODES; ++i) {
        for (int j = 0; j < NUM_NODES; ++j) {
            // For simplicity, set adjacency matrix elements randomly (0 or 1)
            h_adjMatrix[i * NUM_NODES + j] = rand() % 2;
        }
        // Initialize the vector with all zeros except the last element
        h_vector[i] = 0;
    }
    h_vector[NUM_NODES - 1] = 1; // Set the last element to 1 as the starting node

 
    // Perform sequential graph traversal
  

      struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

      sequentialGraphTraversal(h_adjMatrix, h_vector, h_result, NUM_NODES - 1); // Target node is NUM_NODES - 1

    // Record the end time using gettimeofday
    gettimeofday(&end_time, NULL);

    double execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec) / 1000.0;
    printf("\nExecution Time: %f ms\n", execution_time);

    return 0;


    // Free allocated memory
    free(h_adjMatrix);
    free(h_vector);
    free(h_result);

    return 0;
}
