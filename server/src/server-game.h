#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include "../config/config.h"

// Gera um ID único para uma sala.
int generateUniqueId();

void saveRoomStatistics(int roomId, double elapsedTime);

// Carrega um jogo específico a partir do ficheiro 'games.json'.
Game *loadGame(ServerConfig *config, int gameID, int playerID);

// Carrega um jogo aleatório do ficheiro 'games.json'.
Game *loadRandomGame(ServerConfig *config, int playerID);

// Verifica se a linha inserida pelo jogador está correta.
int verifyLine(ServerConfig *config, Game *game, char *solutionSent, int insertLine[9], int playerID);

// Verifica se uma linha do tabuleiro está correta.
bool isLineCorrect(Game *game, int row);

// Cria uma nova sala de jogo.
Room *createRoom(ServerConfig *config, int playerID, bool isSinglePlayer, int synchronizationType);

// Obtém uma sala de jogo a partir do ID.
Room *getRoom(ServerConfig *config, int roomID, int playerID);

// delete room
void deleteRoom(ServerConfig *config, int roomID);

// Obtém uma lista de IDs dos jogos disponíveis.
char *getGames(ServerConfig *config);

// Obtém uma lista das salas de jogo disponíveis.
char *getRooms(ServerConfig *config);

// update game statistics
void updateGameStatistics(ServerConfig *config, int roomID, int elapsedTime, float accuracy);

void acquireReadLock(Room *room);

void releaseReadLock(Room *room);

void acquireWriteLock(Room *room, Client *client);

void releaseWriteLock(Room *room, Client *client);

void acquireTurnsTileSemaphore(Room *room, Client *client);

void releaseTurnsTileSemaphore(Room *room, Client *client);

void enterBarberShop(Room *room, Client *client);

void leaveBarberShop(Room *room, Client *client);

void barberCut(Room *room);

void barberIsDone(Room *room);

void *handleBarber(void *arg);

#endif