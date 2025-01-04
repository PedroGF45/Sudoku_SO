#ifndef SERVER_COMMS_H
#define SERVER_COMMS_H

#include <stdbool.h>
#include "server-game.h"

// Gera um ID único para um cliente.
int generateUniqueClientId();

// Função para lidar com a comunicação com um cliente.
void *handleClient(void *arg);

// Inicializa o socket do servidor e associa-o a um endereço.
void initializeSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig *config);

#endif