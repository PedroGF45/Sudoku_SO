#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int id; 
    char board[9][9];  // Tabuleiro (9x9)
    char solution[9][9];    // Solucao correta
} Game;

typedef struct {
    int id;         // room ID
    int *players;   // has the players IDs
    Game *game;      // game
} Room;

typedef struct {
    int serverPort;
    char gamePath[256];
    char logPath[256];
    int maxPlayers;
    int maxRooms;
    Room **rooms;
} ServerConfig;

// Obter a configuração do servidor
ServerConfig *getServerConfig(char *configPath);

#endif // CONFIG_H