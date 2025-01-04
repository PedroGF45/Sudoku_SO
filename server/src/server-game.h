#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include "../config/config.h"
#include "server-barber.h"
#include "server-barrier.h"
#include "server-readerWriter.h"
#include "server-statistics.h"

// Gera um ID único para uma sala.
int generateUniqueId();

// Verifica se a linha inserida pelo jogador está correta.
int verifyLine(ServerConfig *config, Game *game, char *solutionSent, int insertLine[9], int playerID);

// Verifica se uma linha do tabuleiro está correta.
bool isLineCorrect(Game *game, int row);

// Cria uma sala e um jogo, configurando-os com base nos parâmetros fornecidos.
Room *createRoomAndGame(ServerConfig *config, Client *client, bool isSinglePlayer, bool isRandom, int gameID, int synchronizationType);

// Cria uma nova sala de jogo.
Room *createRoom(ServerConfig *config, int playerID, bool isSinglePlayer, int synchronizationType);

// Obtém uma sala de jogo a partir do ID.
Room *getRoom(ServerConfig *config, int roomID, int playerID);

// Junta um jogador a uma sala existente.
void joinRoom(ServerConfig *config, Room *room, Client *client);

// delete room
void deleteRoom(ServerConfig *config, int roomID);

// Obtém uma lista das salas de jogo disponíveis.
char *getRooms(ServerConfig *config);

// Obtém uma lista de IDs dos jogos disponíveis.
char *getGames(ServerConfig *config);

// Carrega um jogo específico a partir do ficheiro 'games.json'.
Game *loadGame(ServerConfig *config, int gameID, int playerID);

// Carrega um jogo aleatório do ficheiro 'games.json'.
Game *loadRandomGame(ServerConfig *config, int playerID);

// Envia o tabuleiro atual ao cliente em formato JSON.
void sendBoard(ServerConfig *config, Room* room, Client *client);

// Recebe as linhas enviadas pelo cliente e processa-as.
void receiveLines(ServerConfig *config, Room *room, Client *client, int *currentLine);

// Termina o jogo e limpa os recursos associados à sala.
void finishGame(ServerConfig *config, Room *room, int *socket);

// Trata o temporizador de espera do cliente.
void handleTimer(ServerConfig *config, Room *room, Client *client);

// Envia uma mensagem de atualização do temporizador ao cliente.
void sendTimerUpdate(ServerConfig *config, Room *room, Client *client);

#endif