#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "../../utils/logs/logs.h"

ServerConfig *getServerConfig(char *configPath) {

    ServerConfig *config = (ServerConfig *)malloc(sizeof(ServerConfig));
    if (config == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    memset(config, 0, sizeof(ServerConfig));  // Initialize Room struct

    // Abre o ficheiro 'config.txt' em modo de leitura
    FILE *file;
    file = fopen(configPath, "r");

    // Se o ficheiro não existir, imprime uma mensagem de erro e termina o programa
    if (file == NULL) {
        fprintf(stderr, "Couldn't open %s: %s\n", configPath, strerror(errno));
        exit(1);
    }

    char line[256];

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "SERVER_PORT = %d", &config->serverPort);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "GAME_PATH = %s", config->gamePath);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "SERVER_LOG_PATH = %s", config->logPath);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "MAX_ROOMS = %d", &config->maxRooms);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "MAX_PLAYERS_PER_ROOM = %d", &config->maxPlayers);
    }

    // Inicializa as salas
    config->rooms = (Room **)malloc(config->maxRooms * sizeof(Room));
    if (config->rooms == NULL) {
        fprintf(stderr, "Memory allocation failed for rooms\n");
        exit(1);  // Handle the error as appropriate
    }

    // Optionally initialize each Room pointer to NULL
    for (int i = 0; i < config->maxRooms; i++) {
        config->rooms[i] = NULL;  // Initialize each pointer
    }

    // Fecha o ficheiro
    fclose(file);

    writeLogJSON(config->logPath, 0, 0, EVENT_SERVER_START);
    printf("Server Started\n");
    printf("PORTA DO SERVIDOR: %d\n", config->serverPort);
    printf("PATH DO JOGO: %s\n", config->gamePath);
    printf("PATH DO LOG: %s\n", config->logPath);
    printf("MAXIMO DE JOGADORES POR SALA: %d\n", config->maxPlayers);
    printf("MAXIMO DE SALAS: %d\n", config->maxRooms);

    // Retorna a variável config
    return config;
}
