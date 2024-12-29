#ifndef SERVER_COMMS_H
#define SERVER_COMMS_H

#include <stdbool.h>
// Inclui "server-game.h" para aceder à estrutura Game.
#include "server-game.h" 

void sendRoomStatistics(int clientSocket);

// Gera um ID único para um cliente.
int generateUniqueClientId();

// Função para lidar com a comunicação com um cliente.
void *handleClient(void *arg);

// Cria uma sala e um jogo, configurando-os com base nos parâmetros fornecidos.
Room *createRoomAndGame(ServerConfig *config, Client *client, bool isSinglePlayer, bool isRandom, int gameID);

// Junta um jogador a uma sala existente.
void joinRoom(ServerConfig *config, Room *room, Client *client);

// Inicializa o socket do servidor e associa-o a um endereço.
void initializeSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig *config);

// Envia o tabuleiro atual ao cliente em formato JSON.
void sendBoard(ServerConfig *config, Game *game, int *socket);

// Recebe as linhas enviadas pelo cliente e processa-as.
void receiveLines(ServerConfig *config, Room *room, Client *client, int *currentLine);

// Termina o jogo e limpa os recursos associados à sala.
void finishGame(ServerConfig *config, Room *room, int *socket);

// Trata o temporizador de espera do cliente.
void handleTimer(ServerConfig *config, Room *room, Client *client);

// Envia uma mensagem de atualização do temporizador ao cliente.
void sendTimerUpdate(ServerConfig *config, Room *room, Client *client);

#endif