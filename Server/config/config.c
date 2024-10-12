#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"

serverConfig getServerConfig(char *configPath) {

    // Cria uma variável do tipo serverConfig
    serverConfig config;

    //printf("Config path: %s\n", configPath);

    // Abre o ficheiro 'config.txt' em modo de leitura
    FILE *file;
    file = fopen(configPath, "r");

    // Se o ficheiro não existir, imprime uma mensagem de erro e termina o programa
    if (file == NULL) {
        //fprintf(stderr, "Couldn't open %s: %s\n", configPath, strerror(errno));
        exit(1);
    }

    char line[256];
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

    printf("PATH DO JOGO: %s\n", config.gamePath);
    printf("PATH DO LOG: %s\n", config.logPath);

    // Fecha o ficheiro
    fclose(file);

    // Retorna a variável config
    return config;
}