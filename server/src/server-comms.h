#ifndef SERVER_COMMS_H
#define SERVER_COMMS_H

#include <stdbool.h>
// Inclui "server-game.h" para aceder à estrutura Game.
#include "server-game.h" 


// Estrutura que contém dados do cliente, incluindo o descritor de socket e a configuração do servidor.
typedef struct {
    int socket_fd;
    ServerConfig *config;
} ClientData;

// Gera um ID único para um cliente.
int generateUniqueClientId();

// Função para lidar com a comunicação com um cliente.
void *handleClient(void *arg);

// Cria uma sala e um jogo, configurando-os com base nos parâmetros fornecidos.
Room *createRoomAndGame(int *newSockfd, ServerConfig *config, int playerID, bool isSinglePlayer, bool isRandom, int gameID);

// Inicializa o socket do servidor e associa-o a um endereço.
void initializeSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig *config);

// Envia o tabuleiro atual ao cliente em formato JSON.
void sendBoard(int *socket, Game *game, ServerConfig *config);

// Recebe as linhas enviadas pelo cliente e processa-as.
void receiveLines(int *newSockfd, Game *game, int playerID, ServerConfig *config, int *currentLine);

// Termina o jogo e limpa os recursos associados à sala.
void finishGame(int *socket, Room *room, int playerID, ServerConfig *config);


#endif