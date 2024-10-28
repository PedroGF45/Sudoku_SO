#ifndef SERVER_COMMS_H
#define SERVER_COMMS_H

#include "../config/config.h"
#include "jogos.h"

void inicializaSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig config);
void enviarTabuleiro(int socket, Jogo *jogo);
int receberLinhaDoCliente(int socket, char *buffer);

#endif