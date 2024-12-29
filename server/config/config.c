#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "../../utils/logs/logs.h"

/**
 * Lê as configurações do servidor a partir de um ficheiro de configuração e
 * inicializa uma estrutura `ServerConfig` com os valores lidos.
 *
 * @param configPath O caminho para o ficheiro de configuração que contém as definições do servidor.
 * @return Um pointer para uma estrutura `ServerConfig` inicializada, ou NULL se a alocação de memória falhar.
 *
 * @details Esta função faz o seguinte:
 * - Aloca memória para uma estrutura `ServerConfig` e inicializa os seus campos com zeros.
 * - Abre o ficheiro de configuração especificado em modo de leitura.
 * - Lê as definições do ficheiro, como a porta do servidor, o caminho do jogo, o caminho do log, 
 *   o número máximo de salas, e o número máximo de jogadores por sala, 
 *   e preenche os respetivos campos da estrutura.
 * - Aloca memória para um array de pointers de `Room` e inicializa cada pointer a NULL.
 * - Regista o evento de início do servidor no ficheiro de log.
 * - Imprime as configurações do servidor na consola.
 * - Se ocorrer um erro ao abrir o ficheiro ou a alocar memória, 
*    imprime uma mensagem de erro e termina o programa.
 */

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
        sscanf(line, "MAX_PLAYERS_PER_ROOM = %d", &config->maxClientsPerRoom);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "MAX_PLAYERS_ON_SERVER = %d", &config->maxClientsOnline);
    }

    // Fecha o ficheiro
    fclose(file);

    // Inicializa as salas
    config->rooms = (Room **)malloc(config->maxRooms * sizeof(Room));
    if (config->rooms == NULL) {
        fprintf(stderr, "Memory allocation failed for rooms\n");
        exit(1);  // Handle the error as appropriate
    }

    // initialize each Room pointer to NULL
    for (int i = 0; i < config->maxRooms; i++) {
        config->rooms[i] = NULL;  // Initialize each pointer
    }

    // initialize clients
    config->clients = (Client **)malloc(config->maxClientsOnline * sizeof(Client));
    if (config->clients == NULL) {
        fprintf(stderr, "Memory allocation failed for clients\n");
        exit(1);  // Handle the error as appropriate
    }

    // initialize each Client pointer to NULL
    for (int i = 0; i < config->maxClientsOnline; i++) {
        config->clients[i] = NULL;  // Initialize each pointer
    }

    // Inicializa o número de salas e jogadores online
    config->numRooms = 0;
    config->numClientsOnline = 0;

    // Regista o evento de início do servidor no ficheiro de log
    writeLogJSON(config->logPath, 0, 0, EVENT_SERVER_START);

    // Imprime as configurações do servidor na consola
    printf("PORTA DO SERVIDOR: %d\n", config->serverPort);
    printf("PATH DO JOGO: %s\n", config->gamePath);
    printf("PATH DO LOG: %s\n", config->logPath);
    printf("MAXIMO DE JOGADORES POR SALA: %d\n", config->maxClientsPerRoom);
    printf("MAXIMO DE SALAS: %d\n", config->maxRooms);
    printf("MAXIMO DE JOGADORES ONLINE: %d\n", config->maxClientsOnline);

    // Retorna a variável config
    return config;
}

void addClient(ServerConfig *config, Client *client) {
    
    // add client to clients array
    for (int i = 0; i < config->maxClientsOnline; i++) {
        if (config->clients[i] == NULL) {
            config->clients[i] = client;
            break;
        }
    }

    config->numClientsOnline++;
    printf("NUMERO DE JOGADORES ONLINE: %d\n", config->numClientsOnline);
}

void removeClient(ServerConfig *config, Client *client) {
    
    // remove client from clients array
    for (int i = 0; i < config->maxClientsOnline; i++) {
        if (config->clients[i] != NULL && config->clients[i]->socket_fd == client->socket_fd) {
            free(config->clients[i]);
            config->clients[i] = NULL;
            break;
        }
    }

    // shift clients
    for (int i = 0; i < config->maxClientsOnline; i++) {
        if (config->clients[i] == NULL && config->clients[i + 1] != NULL) {
            config->clients[i] = config->clients[i + 1];
            config->clients[i + 1] = NULL;
        }
    }

    // decrement number of clients online
    config->numClientsOnline--;
}
