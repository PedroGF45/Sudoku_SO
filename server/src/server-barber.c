#include <stdio.h>
#include "server-barber.h"

void enterBarberShop(Room *room, Client *client) {

    // initialize self semaphore
    sem_init(&client->selfSemaphore, 0, 0);

    // lock the barber shop mutex
    pthread_mutex_lock(&room->barberShopMutex);

    // increment the number of customers
    room->customers++;

    //printf("CLIENT %d ENTERED THE BARBER SHOP\n", client->clientID);
    // add the client to the barber shop queue
    if (room->priorityQueueType == 0) { // static priority
        enqueueWithPriority(room->barberShopQueue, client->clientID, client->isPremium);
    } else if (room->priorityQueueType == 1) { // dynamic priority
        enqueueWithPriority(room->barberShopQueue, client->clientID, client->isPremium);
        updateQueueWithPriority(room->barberShopQueue, room->maxWaitingTime);
    } else { // FIFO
        enqueueFifo(room->barberShopQueue, client->clientID);
    }
        
    // unlock the barber shop mutex
    pthread_mutex_unlock(&room->barberShopMutex);

    //printf("CLIENT %d IS WAITING FOR THE BARBER1\n", client->clientID);

    // wait for the costumer semaphore
    sem_post(&room->costumerSemaphore);

    //printf("CLIENT %d IS WAITING FOR THE BARBER2\n", client->clientID);

    // wait for the self semaphore to be unlocked by the dequeue function
    sem_wait(&client->selfSemaphore);
}

void leaveBarberShop(Room *room, Client *client) {

    //printf("CLIENT %d IS DONE WITH THE BARBER\n", client->clientID);

    // signal that the costumer is done
    sem_post(&room->costumerDoneSemaphore);

    //printf("CLIENT %d IS WAITING FOR THE BARBER TO BE DONE\n", client->clientID);

    // wait for the barber to be done
    sem_wait(&room->barberDoneSemaphore);

    // lock the barber shop mutex
    pthread_mutex_lock(&room->barberShopMutex);

    // decrement the number of customers
    room->customers--;

    // unlock the barber shop mutex
    pthread_mutex_unlock(&room->barberShopMutex);
}

void barberCut(Room *room) {

    //printf("BARBER IS WAITING FOR A COSTUMER\n");

    

    // wait for the costumer semaphore
    sem_wait(&room->costumerSemaphore);

    // lock the barber shop mutex
    pthread_mutex_lock(&room->barberShopMutex);

    //printf("NUMBER OF CUSTOMERS: %d\n", room->customers);

    // dequeue the client from the barber shop queue
    int clientID = dequeue(room->barberShopQueue);
    Client *client;

    if (clientID == -1) {
        printf("NO CLIENTS IN THE BARBER SHOP\n");
        // unlock the barber shop mutex
        pthread_mutex_unlock(&room->barberShopMutex);
        return;
    }

    // unlock the self semaphore
    for (int i = 0; i < room->maxClients; i++) {
        if (room->clients[i]->clientID == clientID) {
            client = room->clients[i];
            break;
        }
    }

    // unlock the barber shop mutex
    pthread_mutex_unlock(&room->barberShopMutex);

    // post the self semaphore
    sem_post(&client->selfSemaphore);

    // delete the self semaphore
    sem_destroy(&client->selfSemaphore);
}

void barberIsDone(Room *room) {

    //printf("BARBER IS DONE WITH THE COSTUMER1\n");
    // wait for the costumer done semaphore
    sem_wait(&room->costumerDoneSemaphore);

    //printf("BARBER IS DONE WITH THE COSTUMER2\n");

    // signal that the barber is done
    sem_post(&room->barberDoneSemaphore);
}

void *handleBarber(void *arg) {

    while (1) {

        Room *room = (Room *)arg;

        barberCut(room);
        barberIsDone(room);
    }
}