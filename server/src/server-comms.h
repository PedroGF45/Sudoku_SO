#ifndef SERVER_COMMS_H
#define SERVER_COMMS_H

#include <stdbool.h>
#include "server-game.h" // Include server-game.h to access Game struct

typedef struct {
    int socket_fd;
    ServerConfig *config;
} ClientData;

int generateUniqueClientId();

// Função para lidar com o cliente
void *handleClient(void *arg);

// Criar room e jogo
Room *createRoomAndGame(int *newSockfd, ServerConfig *config, int playerID, bool isSinglePlayer, bool isRandom, int gameID);

// Funções de comunicação do servidor
void initializeSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig *config);

// Enviar tabuleiro ao cliente
void sendBoard(int *socket, Game *game, ServerConfig *config);

// Receber linhas do cliente
void receiveLines(int *newSockfd, Game *game, int playerID, ServerConfig *config);

// Terminar jogo
void finishGame(int *socket, Room *room, int playerID, ServerConfig *config);


#endif