#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "../logs/logs.h"
#include "../../utils/logs/logs-common.h"

/**
 * Lê as configurações do servidor a partir de um ficheiro de configuração e
 * inicializa uma estrutura `ServerConfig` com os valores lidos.
 *
 * @param configPath O caminho para o ficheiro de configuração que contém as definições do servidor.
 * @return Um pointer para uma estrutura `ServerConfig` inicializada, ou NULL se a alocação de memória falhar.
 *
 * @details Esta função faz o seguinte:
 * - Aloca memória para uma estrutura `ServerConfig` e inicializa os seus campos com zeros.
 * - Abre o ficheiro de configuração especificado em modo de leitura.
 * - Lê as definições do ficheiro, como a porta do servidor, o caminho do jogo, o caminho do log, 
 *   o número máximo de salas, e o número máximo de jogadores por sala, 
 *   e preenche os respetivos campos da estrutura.
 * - Aloca memória para um array de pointers de `Room` e inicializa cada pointer a NULL.
 * - Regista o evento de início do servidor no ficheiro de log.
 * - Imprime as configurações do servidor na consola.
 * - Se ocorrer um erro ao abrir o ficheiro ou a alocar memória, 
*    imprime uma mensagem de erro e termina o programa.
 */

ServerConfig *getServerConfig(char *configPath) {

    ServerConfig *config = (ServerConfig *)malloc(sizeof(ServerConfig));
    if (config == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    memset(config, 0, sizeof(ServerConfig));  // Initialize Room struct

    // Abre o ficheiro 'config.txt' em modo de leitura
    FILE *file;
    file = fopen(configPath, "r");

    // Se o ficheiro não existir, imprime uma mensagem de erro e termina o programa
    if (file == NULL) {
        fprintf(stderr, "Couldn't open %s: %s\n", configPath, strerror(errno));
        exit(1);
    }

    char line[256];

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "SERVER_PORT = %d", &config->serverPort);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "GAME_PATH = %s", config->gamePath);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "SERVER_LOG_PATH = %s", config->logPath);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "MAX_ROOMS = %d", &config->maxRooms);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "MAX_PLAYERS_PER_ROOM = %d", &config->maxClientsPerRoom);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "MAX_PLAYERS_ON_SERVER = %d", &config->maxClientsOnline);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "MAX_WAITING_TIME = %d", &config->maxWaitingTime);
    }

    // Fecha o ficheiro
    fclose(file);

    // Inicializa as salas
    config->rooms = (Room **)malloc(config->maxRooms * sizeof(Room));
    if (config->rooms == NULL) {
        fprintf(stderr, "Memory allocation failed for rooms\n");
        exit(1);  // Handle the error as appropriate
    }

    // initialize each Room pointer to NULL
    for (int i = 0; i < config->maxRooms; i++) {
        config->rooms[i] = NULL;  // Initialize each pointer
    }

    // initialize clients
    config->clients = (Client **)malloc(config->maxClientsOnline * sizeof(Client));
    if (config->clients == NULL) {
        fprintf(stderr, "Memory allocation failed for clients\n");
        exit(1);  // Handle the error as appropriate
    }

    // initialize each Client pointer to NULL
    for (int i = 0; i < config->maxClientsOnline; i++) {
        config->clients[i] = NULL;  // Initialize each pointer
    }

    // Inicializa o número de salas e jogadores online
    config->numRooms = 0;
    config->numClientsOnline = 0;

    // producer-consumer for writing logs
    sem_init(&config->mutexLogSemaphore, 0, 1); // mutex to grant exclusive access
    sem_init(&config->itemsLogSemaphore, 0, 0); // semaphore to signal when there are items to consume
    sem_init(&config->spacesSemaphore, 0, 10); // semaphore to signal when there are spaces to produce

    // mutex
    pthread_mutex_init(&config->mutex, NULL);

    // produce log message
    writeLogJSON(config->logPath, 0, 0, "Server started");

    // Imprime as configurações do servidor na consola
    printf("PORTA DO SERVIDOR: %d\n", config->serverPort);
    printf("PATH DO JOGO: %s\n", config->gamePath);
    printf("PATH DO LOG: %s\n", config->logPath);
    printf("MAXIMO DE JOGADORES POR SALA: %d\n", config->maxClientsPerRoom);
    printf("MAXIMO DE SALAS: %d\n", config->maxRooms);
    printf("MAXIMO DE JOGADORES ONLINE: %d\n", config->maxClientsOnline);
    printf("MAXIMO DE TEMPO DE ESPERA: %d\n", config->maxWaitingTime);

    // Retorna a variável config
    return config;
}

void addClient(ServerConfig *config, Client *client) {
    
    // add client to clients array
    for (int i = 0; i < config->maxClientsOnline; i++) {
        if (config->clients[i] == NULL) {
            config->clients[i] = client;
            break;
        }
    }

    config->numClientsOnline++;
    //printf("NUMERO DE JOGADORES ONLINE: %d\n", config->numClientsOnline);
}

void removeClient(ServerConfig *config, Client *client) {
    
    // remove client from clients array
    for (int i = 0; i < config->maxClientsOnline; i++) {
        if (config->clients[i] != NULL && config->clients[i]->socket_fd == client->socket_fd) {
            free(config->clients[i]);
            config->clients[i] = NULL;
            break;
        }
    }

    // shift clients
    for (int i = 0; i < config->maxClientsOnline; i++) {
        if (config->clients[i] == NULL && config->clients[i + 1] != NULL) {
            config->clients[i] = config->clients[i + 1];
            config->clients[i + 1] = NULL;
        }
    }

    // decrement number of clients online
    config->numClientsOnline--;
}

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

    sem_wait(&queue->empty); // wait for empty space

    //printf("CLIENT %d JOINING THE QUEUE\n", clientID);

    pthread_mutex_lock(&queue->mutex); // lock the queue

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

    sem_wait(&queue->empty); // wait for empty space

    pthread_mutex_lock(&queue->mutex); // lock the queue

    if (queue->front == NULL) { // if the queue is empty
        queue->front = newNode;
        queue->rear = newNode;
    } else {                    // if the queue is not empty
        queue->rear->next = newNode;
        queue->rear = newNode;
    }

    pthread_mutex_unlock(&queue->mutex);

    sem_post(&queue->full);

}


int dequeue(PriorityQueue *queue) {
    //printf("CLIENT WANTS TO BE REMOVED FROM THE QUEUE\n");

    sem_wait(&queue->full); // wait for full queue

    //printf("CLIENT REMOVING FROM THE QUEUE\n");

    pthread_mutex_lock(&queue->mutex); // lock the queue

    Node *temp = queue->front;

    if (queue->front == NULL) {
        printf("Queue is empty\n");
        pthread_mutex_unlock(&queue->mutex);
        sem_post(&queue->empty);
        return -1;
    }

    queue->front = queue->front->next;

    if (queue->front == NULL) { // if the queue is empty
        queue->rear = NULL;
    }

    int clientID = temp->clientID;

    free(temp);

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->empty);
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
