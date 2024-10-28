#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "../../utils/logs/logs.h"

ServerConfig getServerConfig(char *configPath) {

    // Cria uma variável do tipo clientConfig
    ServerConfig config;

    //printf("Config path: %s\n", configPath);

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
        sscanf(line, "SERVER_PORT = %d", &config.serverPort);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "GAME_PATH = %s", config.gamePath);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "SERVER_LOG_PATH = %s", config.logPath);
    }

    // Fecha o ficheiro
    fclose(file);

    writeLogJSON(config.logPath, 0, 0, EVENT_SERVER_START);
    printf("Server Started\n");
    printf("PORTA DO SERVIDOR: %d\n", config.serverPort);
    printf("PATH DO JOGO: %s\n", config.gamePath);
    printf("PATH DO LOG: %s\n", config.logPath);

    // Retorna a variável config
    return config;
}