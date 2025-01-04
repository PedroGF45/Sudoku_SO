#include <stdio.h>
#include "server-readerWriter.h"

void acquireReadLock(Room *room) {
    // indicate that a reader is entering the room
    sem_wait(&room->readSemaphore);

    // lock mutex
    pthread_mutex_lock(&room->readMutex);

    // increment reader count
    room->readerCount++;

    // if first reader lock the write semaphore
    if (room->readerCount == 1) {
        sem_wait(&room->writeSemaphore); //(decrementa o semáforo)
    }

    // unlock mutex
    pthread_mutex_unlock(&room->readMutex);

    // unlock the reader semaphore
    sem_post(&room->readSemaphore);
}

void releaseReadLock(Room *room) {
    // lock mutex
    pthread_mutex_lock(&room->readMutex);

    // decrement reader count
    room->readerCount--;

    // if last reader unlock the write semaphore
    if (room->readerCount == 0) {
        sem_post(&room->writeSemaphore);//(incrementa o semáforo)
    }

    // unlock mutex
    pthread_mutex_unlock(&room->readMutex);
}

void acquireWriteLock(Room *room, Client *client) {

    // add a random delay to appear more natural
    //int delay = rand() % 5;
    //sleep(delay);

    pthread_mutex_lock(&room->writeMutex);

    // increment writer count
    room->writerCount++;

    //printf("WRITER COUNT: %d from client %d\n", room->writerCount, client->clientID);

    // if first writer lock the read semaphore to block readers
    if (room->writerCount == 1) {
        //printf("WRITER %d IS LOCKING READ SEMAPHORE\n", client->clientID);
        sem_wait(&room->readSemaphore);
    }

    // unlock the writer mutex
    pthread_mutex_unlock(&room->writeMutex);

    // lock the write semaphore
    //printf("WRITER %d IS WAITING FOR WRITE SEMAPHORE\n", client->clientID);
    sem_wait(&room->writeSemaphore);
}

void releaseWriteLock(Room *room, Client *client) {

    sem_post(&room->writeSemaphore);

    // lock the writer mutex
    pthread_mutex_lock(&room->writeMutex);

    // decrement writer count
    room->writerCount--;

    // if last writer unlock the read semaphore
    if (room->writerCount == 0) {
        sem_post(&room->readSemaphore);
    }

    //printf("WRITER %d IS DONE WRITING\n", client->clientID);

    // unlock the writer mutex
    pthread_mutex_unlock(&room->writeMutex);

}