#ifndef SERVER_BARBER_H
#define SERVER_BARBER_H

#include "../config/config.h"

void enterBarberShop(Room *room, Client *client);

void leaveBarberShop(Room *room, Client *client);

void barberCut(Room *room);

void barberIsDone(Room *room);

void *handleBarber(void *arg);

#endif // SERVER_BARBER_H