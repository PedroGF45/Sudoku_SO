#ifndef CLIENT_COMMS_H
#define CLIENT_COMMS_H

#include <stdbool.h>
#include "../../utils/network/network.h"
#include "../config/config.h"

// Estabelece uma ligação TCP com o servidor.
void connectToServer(struct sockaddr_in *serv_addr, int *socketfd, clientConfig *config);

// Envia uma mensagem para fechar a conexão com o servidor.
void closeConnection(int *socketfd, clientConfig *config);

#endif // CLIENT_COMMS_H