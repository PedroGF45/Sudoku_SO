#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"

clientConfig getClientConfig(char *configPath) {

    // Cria uma variável do tipo serverConfig
    clientConfig config;

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
        sscanf(line, "SERVER_IP = %s", config.serverIP);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "SERVER_PORT = %d", &config.serverPort);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "SERVER_HOSTNAME = %s", config.serverHostName);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "LOG_PATH = %s", config.logPath);
    }
    
    if (fgets(line, sizeof(line), file) != NULL) {
        int isManual;
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "IS_MANUAL = %d", &isManual);

        // Converte o valor lido para booleano
        config.isManual = isManual == 1 ? true : false;
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "DIFFICULTY = %d", &config.difficulty);
    }

    // Fecha o ficheiro
    fclose(file);

    printf("IP do servidor: %s\n", config.serverIP);
    printf("Porta do servidor: %d\n", config.serverPort);
    printf("Hostname do servidor: %s\n", config.serverHostName);
    printf("ID do cliente: %d\n", config.clientID);
    printf("Log path do cliente: %s\n", config.logPath);
    printf("Modo: %s\n", config.isManual ? "manual" : "automatico");
    if (config.difficulty == 1) {
        printf("Dificuldade: Facil\n");
    } else if (config.difficulty == 2) {
        printf("Dificuldade: Medio\n");
    } else {
        printf("Dificuldade: Dificil\n");
    }

    // Retorna a variável config
    return config;
}