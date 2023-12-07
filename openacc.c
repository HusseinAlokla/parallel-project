

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_NODES 1005

void bfsTraversal(int *adjMatrix, int *visited, int *queue, int *marked, int queueSize, int *newQueueSize)
{
#pragma acc parallel loop present(adjMatrix, visited, queue, marked)
    for (int idx = 0; idx < queueSize; idx++)
    {
        int node = queue[idx];
        for (int i = 0; i < NUM_NODES; i++)
        {
            if (adjMatrix[node * NUM_NODES + i] == 1 && !visited[i])
            {
                marked[i] = 1; // Mark node as to be visited
            }
        }
    }

    // Sequentially update the queue based on marked array
    int localQueueSize = queueSize;
    for (int i = 0; i < NUM_NODES; i++)
    {
        if (marked[i] && !visited[i])
        {
            queue[localQueueSize++] = i; // Enqueue
            visited[i] = 1;              // Mark as visited
        }
    }
    *newQueueSize = localQueueSize; // Update the new queue size
}

int main()
{
    int h_adjMatrix[NUM_NODES * NUM_NODES];
    int h_visited[NUM_NODES] = {0};
    int h_queue[NUM_NODES * NUM_NODES] = {0};
    int h_marked[NUM_NODES] = {0};
    int queueSize = 1;
    int newQueueSize = 0;

    // Initialize the adjacency matrix and vector
    for (int i = 0; i < NUM_NODES; ++i)
    {
        for (int j = 0; j < NUM_NODES; ++j)
        {
            h_adjMatrix[i * NUM_NODES + j] = rand() % 2;
        }
    }

    int startNode = 0;
    h_visited[startNode] = 1;
    h_queue[0] = startNode;

    struct timeval start, end;

    gettimeofday(&start, NULL);
// OpenACC data region
#pragma acc data copy(h_adjMatrix, h_visited, h_queue, h_marked) copyin(queueSize) copyout(newQueueSize)
    {
        bfsTraversal(h_adjMatrix, h_visited, h_queue, h_marked, queueSize, &newQueueSize);
    }

    gettimeofday(&end, NULL);

    long seconds = (end.tv_sec - start.tv_sec);
    long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);

    printf("\nExecution Time: %f ms\n", micros / 1000.0);
    return 0;
}
