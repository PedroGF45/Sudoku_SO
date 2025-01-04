#ifndef SERVER_BARRIER_H
#define SERVER_BARRIER_H

#include "../config/config.h"

void acquireTurnsTileSemaphore(Room *room, Client *client);

void releaseTurnsTileSemaphore(Room *room, Client *client);

#endif // SERVER_BARRIER_H