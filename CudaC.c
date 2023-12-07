
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

#define NUM_NODES 1004

__global__ void bfsTraversal(int *adjMatrix, int *visited, int *queue, int *result, int queueSize)
{
    int threadId = blockIdx.x * blockDim.x + threadIdx.x; // Calculate the global thread ID
    if (threadId < queueSize)
    {                               // Check if the thread ID is within the queue size
        int node = queue[threadId]; // Get the node to be processed by this thread
        for (int i = 0; i < NUM_NODES; i++)
        { // Loop through all nodes to find adjacent nodes
            if (adjMatrix[node * NUM_NODES + i] == 1 && !visited[i])
            {                                 // If an adjacent node is found and not visited
                visited[i] = 1;               // Mark it as visited
                result[i] = result[node] + 1; // Update the result for that node
                queue[queueSize + i] = i;     // Enqueue
            }
        }
    }
}

int main()
{
    // Host matrices and vectors
    int h_adjMatrix[NUM_NODES * NUM_NODES];   // Host memory for adjacency matrix
    int h_visited[NUM_NODES] = {0};           // Host memory for visited nodes array, initialized to 0
    int h_queue[NUM_NODES * NUM_NODES] = {0}; // Host memory for queue, initialized to 0
    int h_result[NUM_NODES] = {0};            // Host memory for result array, initialized to 0

    // Initialize the adjacency matrix and vector
    for (int i = 0; i < NUM_NODES; ++i)
    { // Loop to initialize adjacency matrix
        for (int j = 0; j < NUM_NODES; ++j)
        {
            h_adjMatrix[i * NUM_NODES + j] = rand() % 2; // Randomly set adjacency matrix elements to 0 or 1
        }
    }

    // Set the starting node
    int startNode = 0;        // Define the start node
    h_visited[startNode] = 1; // Mark the start node as visited
    h_queue[0] = startNode;   // Enqueue the start node
    int queueSize = 1;        // Set initial queue size to 1

    // Device matrices and vectors
    int *d_adjMatrix, *d_visited, *d_queue, *d_result; // Device memory pointers

    // Allocate device memory
    cudaMalloc(&d_adjMatrix, NUM_NODES * NUM_NODES * sizeof(int)); // Allocate memory for adjacency matrix on device
    cudaMalloc(&d_visited, NUM_NODES * sizeof(int));               // Allocate memory for visited nodes array on device
    cudaMalloc(&d_queue, NUM_NODES * NUM_NODES * sizeof(int));     // Allocate memory for queue on device
    cudaMalloc(&d_result, NUM_NODES * sizeof(int));                // Allocate memory for result array on device

    // Copy data from host to device
    cudaMemcpy(d_adjMatrix, h_adjMatrix, NUM_NODES * NUM_NODES * sizeof(int), cudaMemcpyHostToDevice); // Copy adjacency matrix to device
    cudaMemcpy(d_visited, h_visited, NUM_NODES * sizeof(int), cudaMemcpyHostToDevice);                 // Copy visited array to device
    cudaMemcpy(d_queue, h_queue, NUM_NODES * NUM_NODES * sizeof(int), cudaMemcpyHostToDevice);         // Copy queue to device
    cudaMemcpy(d_result, h_result, NUM_NODES * sizeof(int), cudaMemcpyHostToDevice);                   // Copy result array to device

    // Define block and grid dimensions
    int blockSize = 256;                                    // Number of threads in each block
    int gridSize = (queueSize + blockSize - 1) / blockSize; // Number of blocks in the grid

    // Create CUDA events for timing
    cudaEvent_t start, stop; // CUDA event objects for timing
    cudaEventCreate(&start); // Create a start event
    cudaEventCreate(&stop);  // Create a stop event

    // Record the start event
    cudaEventRecord(start, 0); // Record the start time

    // Launch the BFS traversal kernel on the GPU
    bfsTraversal<<<gridSize, blockSize>>>(d_adjMatrix, d_visited, d_queue, d_result, queueSize); // Launch kernel

    // Record the stop event
    cudaEventRecord(stop, 0); // Record the stop time

    // Synchronize and copy results back to host
    cudaDeviceSynchronize();                                                           // Synchronize device to ensure kernel completion
    cudaMemcpy(h_visited, d_visited, NUM_NODES * sizeof(int), cudaMemcpyDeviceToHost); // Copy visited array back to host
    cudaMemcpy(h_result, d_result, NUM_NODES * sizeof(int), cudaMemcpyDeviceToHost);   // Copy result array back to host

    // Calculate elapsed time
    float elapsedTime;
    cudaEventElapsedTime(&elapsedTime, start, stop);    // Calculate the elapsed time
    printf("\n\nExecution Time: %f ms\n", elapsedTime); // Print the execution time

    // Free device memory
    cudaFree(d_adjMatrix); // Free device memory for adjacency matrix
    cudaFree(d_visited);   // Free device memory for visited array
    cudaFree(d_queue);     // Free device memory for queue
    cudaFree(d_result);    // Free device memory for result array

    // Destroy CUDA events
    cudaEventDestroy(start); // Destroy the start event
    cudaEventDestroy(stop);  // Destroy the stop event

    return 0; // Return from main function
}
