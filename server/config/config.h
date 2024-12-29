#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

/**
 * Estrutura que representa um jogo, incluindo o tabuleiro e a solução correta.
 *
 * @param id O identificador único do jogo.
 * @param board O tabuleiro de jogo, representado como uma matriz de 9x9.
 * @param solution A solução correta do jogo, também representada como uma matriz de 9x9.
 * @param currentLine O número da linha atual a resolver no jogo.
 */

typedef struct {
    int id;        
    char board[9][9];
    char solution[9][9];
    int currentLine;
} Game;

// Estrutura que contém dados do cliente, incluindo o descritor de socket e a configuração do servidor.
typedef struct {
    int socket_fd;
    int clientID;
    bool isPremium;
    bool startAgain;
} Client;

/**
 * Estrutura que representa uma sala de jogo.
 *
 * @param id O identificador único da sala.
 * @param maxClients O número máximo de jogadores que a sala pode conter.
 * @param numClients O número atual de jogadores na sala.
 * @param Clients Um pointer para um array que contém os IDs dos jogadores na sala.
 * @param game Um pointer para o jogo associado à sala.
 */

typedef struct {
    int id;
    int maxClients;
    int numClients;
    Client **clients;
    int timer;
    bool isGameRunning;
    bool isSinglePlayer;
    bool isFinished;
    Game *game;
    time_t startTime;
    double elapsedTime;
    pthread_mutex_t timerMutex;
    pthread_mutex_t mutex;
    sem_t beginSemaphore;
    int *premiumQueue;           // Fila de IDs de jogadores premium
    int *nonPremiumQueue;        // Fila de IDs de jogadores não premium
    int premiumQueueSize;        // Tamanho da fila de jogadores premium
    int nonPremiumQueueSize;     // Tamanho da fila de jogadores não premium
    sem_t premiumSemaphore;      // Semáforo para jogadores premium
    sem_t nonPremiumSemaphore;   // Semáforo para jogadores não premium
} Room;


/**
 * Estrutura que representa a configuração do servidor.
 *
 * @param serverPort O número da porta que o servidor utiliza para comunicação.
 * @param gamePath O caminho para o ficheiro que contém os dados do jogo.
 * @param logPath O caminho para o ficheiro onde os logs do servidor são guardados.
 * @param maxRooms O número máximo de salas que o servidor pode gerir.
 * @param maxClientsPerRoom O número máximo de jogadores que cada sala pode conter.
 * @param numRooms O número atual de salas criadas no servidor.
 * @param rooms Um pointer para um array de pointers de `Room`, 
 * que representa as salas de jogo geridas pelo servidor.
 */

typedef struct {
    int serverPort;
    char gamePath[256];
    char logPath[256];
    int maxRooms;
    int maxClientsPerRoom;
    int maxClientsOnline;
    int numClientsOnline;
    int numRooms;
    Room **rooms;
    Client **clients;
} ServerConfig;

typedef struct {
    Client *client;
    ServerConfig *config;
} client_data;


// Obter a configuração do servidor
ServerConfig *getServerConfig(char *configPath);

// Adicionar um cliente à lista de clientes online
void addClient(ServerConfig *config, Client *client);

// Remover um cliente da lista de clientes online
void removeClient(ServerConfig *config, Client *client);

#endif // CONFIG_H