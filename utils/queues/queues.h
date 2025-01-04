#ifndef QUEUES_H
#define QUEUES_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct Node {
    int clientID;
    int timeInQueue;
    bool isPremium;
    struct Node* next;
} Node;

// Define a structure for the queue
typedef struct PriorityQueue {
    Node* front;
    Node* rear;
    pthread_mutex_t mutex;
    sem_t empty;
    sem_t full;
} PriorityQueue;

// create a new node
Node *createNode(int clientID, bool isPremium);

// initialize the queue
void initPriorityQueue(PriorityQueue *queue, int queueSize);

// enqueue an element
void enqueueWithPriority(PriorityQueue *queue, int clientID, bool isPremium);

// update the priority of the elements in the queue
void updatePriority(PriorityQueue *queue);

// update the queue with the priority
void updateQueueWithPriority(PriorityQueue *queue, int maxWaitingTime);

// enqueue an element in a FIFO way
void enqueueFifo(PriorityQueue *queue, int clientID);

// dequeue an element
int dequeue(PriorityQueue *queue);

// free the queue
void freePriorityQueue(PriorityQueue *queue);

#endif // QUEUES_H