#include <stdio.h>
#include "server-barrier.h"

void acquireTurnsTileSemaphore(Room *room, Client *client) {

    // lock the mutex
    sem_wait(&room->mutexSemaphore);

    //printf("CLIENT %d ARRIVED AT THE TILE SEMAPHORE\n", client->clientID);

    // increment the waiting count
    room->waitingCount++;

    // if all players are waiting unlock the turns tile semaphore
    if (room->waitingCount == room->numClients) {

        //printf("CLIENT %d ARRIVED AND IT'S THE LAST ONE\n", client->clientID);
        //printf("RELEASING TURNS TILE SEMAPHORE\n");

        // need to loop n times to unlock the turns tile semaphore
        for (int i = 0; i < room->numClients; i++) {
            sem_post(&room->turnsTileSemaphore1);
        }
    }

    // unlock the mutex
    sem_post(&room->mutexSemaphore);

    //printf("CLIENT %d IS WAITING FOR TURNS TILE SEMAPHORE\n", client->clientID);
    // wait for the turns tile semaphore
    sem_wait(&room->turnsTileSemaphore1);

    //printf("CLIENT %d IS DONE WAITING FOR TURNS TILE SEMAPHORE\n", client->clientID);
}

void releaseTurnsTileSemaphore(Room *room, Client *client) {

    // lock the mutex
    sem_wait(&room->mutexSemaphore);

    //printf("CLIENT %d ARRIVED AT THE TILE SEMAPHORE\n", client->clientID);

    // decrement the waiting count
    room->waitingCount--;

    // if all players are done unlock the turns tile semaphore
    if (room->waitingCount == 0) {

        //printf("CLIENT %d ARRIVED AND IT'S THE LAST ONE\n", client->clientID);
        //printf("RELEASING TURNS TILE SEMAPHORE\n");

        // need to loop n times to unlock the turns tile semaphore
        for (int i = 0; i < room->maxClients; i++) {
            sem_post(&room->turnsTileSemaphore2);
        }
    }

    // unlock the mutex
    sem_post(&room->mutexSemaphore);

    //printf("CLIENT %d IS WAITING FOR TURNS TILE SEMAPHORE\n", client->clientID);
    // wait for the turns tile semaphore
    sem_wait(&room->turnsTileSemaphore2);
    //printf("CLIENT %d IS DONE WAITING FOR TURNS TILE SEMAPHORE\n", client->clientID);

}