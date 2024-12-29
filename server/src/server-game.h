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
int verifyLine(char *logFileName, char *solutionSent, Game *game, int insertLine[9], int playerID);

// Verifica se uma linha do tabuleiro está correta.
bool isLineCorrect(Game *game, int row);

// Cria uma nova sala de jogo.
Room *createRoom(ServerConfig *config, int playerID, bool isSinglePlayer);

// Obtém uma sala de jogo a partir do ID.
Room *getRoom(ServerConfig *config, int roomID, int playerID);

void deleteRoom(ServerConfig *config, int roomID);

// Obtém uma lista de IDs dos jogos disponíveis.
char *getGames(ServerConfig *config);

// Obtém uma lista das salas de jogo disponíveis.
char *getRooms(ServerConfig *config);

// update game statistics
void updateGameStatistics(ServerConfig *config, int roomID, int elapsedTime, float accuracy);

#endif