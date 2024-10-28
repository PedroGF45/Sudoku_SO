#ifndef SERVER_COMMS_H
#define SERVER_COMMS_H

#include "../config/config.h"
#include "server-game.h" // Include server-game.h to access Game struct

// Funções de comunicação do servidor
void initializeSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig config);

// Enviar tabuleiro ao cliente
void sendBoard(int *socket, Game *game);

// Receber linhas do cliente
void receiveLines(int *newSockfd, Game *game, int playerID, ServerConfig config);

// Receber linha do cliente
int receiveLine(int *socket, char *buffer);

#endif