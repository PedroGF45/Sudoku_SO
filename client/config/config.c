#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"




/**
 * Carrega as configurações do cliente a partir de um ficheiro de configuração especificado.
 *
 * @param configPath O caminho para o ficheiro de configuração que contém as definições do cliente.
 * @return Uma estrutura `clientConfig` preenchida com as configurações do cliente.
 *
 * @details A função realiza as seguintes operações:
 * - Abre o ficheiro de configuração em modo de leitura. Se o ficheiro não puder ser aberto, o programa termina.
 * - Lê cada linha do ficheiro e extrai as configurações usando `sscanf`:
 *   - Endereço IP do servidor (`SERVER_IP`).
 *   - Porta do servidor (`SERVER_PORT`).
 *   - Nome do host do servidor (`SERVER_HOSTNAME`).
 *   - Caminho para o ficheiro de log (`LOG_PATH`).
 *   - Modo de jogo (manual ou automático) convertido para booleano (`IS_MANUAL`).
 *   - Nível de dificuldade do jogo (`DIFFICULTY`).
 * - Remove caracteres de nova linha de cada linha lida para garantir que os dados são processados corretamente.
 * - Imprime as configurações carregadas no terminal.
 * - Fecha o ficheiro de configuração e retorna a estrutura `clientConfig`.
 */

clientConfig *getClientConfig(char *configPath) {

    // Cria uma variável do tipo clientConfig
    clientConfig *config = (clientConfig *)malloc(sizeof(clientConfig));
    if (config == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    memset(config, 0, sizeof(clientConfig));  // Inicializa a estrutura clientConfig

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
        sscanf(line, "SERVER_IP = %s", config->serverIP);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "SERVER_PORT = %d", &config->serverPort);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "SERVER_HOSTNAME = %s", config->serverHostName);
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "LOG_PATH = %s", config->logPath);
    }
    
    if (fgets(line, sizeof(line), file) != NULL) {
        int isManual;
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "IS_MANUAL = %d", &isManual);

        // Converte o valor lido para booleano
        config->isManual = isManual == 1 ? true : false;
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        int isPremium;
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "IS_PREMIUM = %d", &isPremium);

        // Converte o valor lido para booleano
        config->isPremium = isPremium == 1 ? true : false;
    }

    if (fgets(line, sizeof(line), file) != NULL) {
        // Remover a nova linha, se houver
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "DIFFICULTY = %d", &config->difficulty);
    }

    // id to 0
    config->clientID = 0;

    // Fecha o ficheiro
    fclose(file);

    printf("IP do servidor: %s\n", config->serverIP);
    printf("Porta do servidor: %d\n", config->serverPort);
    printf("Hostname do servidor: %s\n", config->serverHostName);
    printf("Log path do cliente: %s\n", config->logPath);
    printf("Modo: %s\n", config->isManual ? "manual" : "automatico");
    printf("Cliente %s premium.\n", config->isPremium ? "SIM" : "NAO");

    if (config->difficulty == 1) {
        printf("Dificuldade: Facil\n");
    } else if (config->difficulty == 2) {
        printf("Dificuldade: Medio\n");
    } else {
        printf("Dificuldade: Dificil\n");
    }

    // Retorna a variável config
    return config;
}