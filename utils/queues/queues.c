#include <stdlib.h>
#include <stdio.h>
#include "queues.h"

Node *createNode(int clientID, bool isPremium) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    newNode->clientID = clientID;
    newNode->isPremium = isPremium;
    newNode->next = NULL;
    newNode->timeInQueue = 0;
    return newNode;
}

void initPriorityQueue(PriorityQueue *queue, int queueSize) {
    queue->front = NULL;
    queue->rear = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    sem_init(&queue->empty, 0, queueSize);
    sem_init(&queue->full, 0, 0);
}

void enqueueWithPriority(PriorityQueue *queue, int clientID, bool isPremium) {
    //printf("CLIENT %d WANT TO JOIN THE ROOM\n", clientID);

    Node *newNode = createNode(clientID, isPremium); // create a new node
    //printf("CREATING A NODE FOR CLIENt %d WHICH IS %s\n", clientID, isPremium ? "PREMIUM" : "NOT PREMIUM");

    sem_wait(&queue->empty); // wait for empty space (fica a espera que haja espaço na fila para adicionar um novo cliente)

    //printf("CLIENT %d JOINING THE QUEUE\n", clientID);

    pthread_mutex_lock(&queue->mutex); // lock the queue (garante a exclusão mútua se dois clientes tentarem aceder simultaneamente à fila)

    if (queue->front == NULL) { // if the queue is empty
        //printf("QUEUE IS EMPTY: ADDING CLIENT %d TO THE FIRST NODE\n", clientID);
        queue->front = newNode;
        queue->rear = newNode;
    } else {                    // if the queue is not empty

        //print order of the queue
        //Node *temp1 = queue->front;
        //printf("--------------------\n");
        //printf("QUEUE ORDER BEFORE ENQUEING:\n");
        //while (temp1 != NULL) {
        //    printf("Client %d isPremium: %s\n", temp1->clientID, temp1->isPremium ? "true" : "false");
        //    temp1 = temp1->next;
        //}
        //printf("--------------------\n");
        // order by isPremium
        Node *temp = queue->front;
        Node *prev = NULL;

        // find last premium
        while (temp != NULL && temp->isPremium) {
            prev = temp;
            temp = temp->next;
        }

        // if not premium, find last non-premium
        if (!isPremium) {
            while (temp != NULL && !temp->isPremium) {
                prev = temp;
                temp = temp->next;
            }
        }

        if (prev == NULL) { // if the new node is the first
            //printf("CLIENT %d IS THE FIRST NODE\n", clientID);
            newNode->next = queue->front;
            queue->front = newNode;
        } else if (temp == NULL) { // if the new node is the last
            //printf("CLIENT %d IS THE LAST NODE\n", clientID);
            queue->rear->next = newNode;
            queue->rear = newNode;
        } else { // if the new node is in the middle
            //printf("CLIENT %d IS IN THE MIDDLE\n", clientID);
            prev->next = newNode;
            newNode->next = temp;
        }
    }

    // print order of the queue
    //Node *temp2 = queue->front;
    //printf("--------------------\n");
    //printf("QUEUE ORDER AFTER ENQUEUING:\n");
    //while (temp2 != NULL) {
    //    printf("Client %d isPremium: %s\n", temp2->clientID, temp2->isPremium ? "true" : "false");
    //    temp2 = temp2->next;
    //}
    //printf("FRONT: %d\n", queue->front->clientID);
    //printf("REAR: %d\n", queue->rear->clientID);
    //printf("--------------------\n");

    // increment timeonQueue for all clients in the queue
    updatePriority(queue);

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->full);
}

void updatePriority(PriorityQueue *queue) {
    Node *temp = queue->front;
    while (temp != NULL) {
        temp->timeInQueue++;
        temp = temp->next;
    }
}

void updateQueueWithPriority(PriorityQueue *queue, int maxWaitingTime) {

    // lock the queue
    pthread_mutex_lock(&queue->mutex);

    // print order of the queue
    //Node *temp1 = queue->front;
    //printf("--------------------\n");
    //printf("QUEUE ORDER BEFORE UPDATEING PRIORITY:\n");
    //while (temp1 != NULL) {
    //    printf("Client %d isPremium: %s TIME:%d\n", temp1->clientID, temp1->isPremium ? "true" : "false", temp1->timeInQueue);
    //    temp1 = temp1->next;
    //}
    //printf("FRONT: %d\n", queue->front->clientID);
    //printf("REAR: %d\n", queue->rear->clientID);
    //printf("--------------------\n");

    Node *temp = queue->front;
    Node *prev = NULL;
    while (temp != NULL) {
        // if time in queue has reached the max waiting time put it in the front of the queue
        if (temp->timeInQueue >= maxWaitingTime) {

            // reset time in queue
            temp->timeInQueue = 0;

            // if the node is already in the front of the queue
            if (prev == NULL) {
                temp = temp->next; // move to the next node
            } else if (temp->next == NULL) { // if the node is the last node
                prev->next = NULL;              //
                temp->next = queue->front;
                queue->front = temp;
                queue->rear = prev;
            } else { // if the node is in the middle of the queue
                prev->next = temp->next;
                temp->next = queue->front;
                queue->front = temp;
                temp = prev->next;
            }

        } else { // move to the next node
            prev = temp;
            temp = temp->next;
        }
            
    }

    // print order of the queue
    //Node *temp2 = queue->front;
    //printf("--------------------\n");
    //printf("QUEUE ORDER AFTER UPDATING PRIORITY:\n");
    //while (temp2 != NULL) {
    //    printf("Client %d isPremium: %s TIME:%d\n", temp2->clientID, temp2->isPremium ? "true" : "false", temp2->timeInQueue);
    //    temp2 = temp2->next;
    //}
    //printf("FRONT: %d\n", queue->front->clientID);
    //printf("REAR: %d\n", queue->rear->clientID);
    //printf("--------------------\n");


    // unlock the queue
    pthread_mutex_unlock(&queue->mutex);
}

void enqueueFifo(PriorityQueue *queue, int clientID) {

    Node *newNode = createNode(clientID, false); // create a new node

    sem_wait(&queue->empty); // wait for empty space(se for >0 continua a decrementar se for =0 fica a espera que haja espaço na fila para adicionar um novo cliente)

    pthread_mutex_lock(&queue->mutex); // lock the queue

    if (queue->front == NULL) { // if the queue is empty(ao adicionar 1 cliente passa para a função de baixo porque a  fila já não está vazia)
        queue->front = newNode;
        queue->rear = newNode;
    } else {                    // if the queue is not empty(adiciona o cliente no fim da fila)
        queue->rear->next = newNode;
        queue->rear = newNode;
    }

    pthread_mutex_unlock(&queue->mutex);

    sem_post(&queue->full);//(incrementa o semáforo full, indicando que há um item na fila para ser consumido)

}


int dequeue(PriorityQueue *queue) {
    //printf("CLIENT WANTS TO BE REMOVED FROM THE QUEUE\n");

    sem_wait(&queue->full); // wait for full queue (decr)

    //printf("CLIENT REMOVING FROM THE QUEUE\n");

    pthread_mutex_lock(&queue->mutex); // lock the queue

    Node *temp = queue->front;//(guarda o primeiro nó em temp do cliente que vai ser removido)

    if (queue->front == NULL) {
        printf("Queue is empty\n");
        pthread_mutex_unlock(&queue->mutex);
        sem_post(&queue->empty);
        return -1;
    }

    queue->front = queue->front->next;//(avança o front para o próximo cliente)

    if (queue->front == NULL) { // if the queue is empty
        queue->rear = NULL;
    }

    int clientID = temp->clientID;

    free(temp);

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->empty);// incrementa o semáforo empty, indicando que há um espaço na fila para ser preenchido
    //printf("CLIENT REMOVED FROM THE QUEUE\n");

    return clientID;
}

void freePriorityQueue(PriorityQueue *queue) {
    Node *temp = queue->front;
    while (temp != NULL) {
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
    pthread_mutex_destroy(&queue->mutex);
    sem_destroy(&queue->empty);
    sem_destroy(&queue->full);
    free(queue);
}
