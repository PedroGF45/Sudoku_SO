#ifndef SERVER_READERWRITER_H
#define SERVER_READERWRITER_H

#include "../config/config.h"

void acquireReadLock(Room *room);

void releaseReadLock(Room *room);

void acquireWriteLock(Room *room, Client *client);

void releaseWriteLock(Room *room, Client *client);

#endif // SERVER_READERWRITER_H
