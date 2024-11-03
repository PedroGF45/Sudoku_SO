#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/**
 * Estrutura que armazena as configurações do cliente, incluindo informações de rede, identificação, 
 * e opções de jogo.
 */
typedef struct {
    char serverIP[256];         /**< Endereço IP do servidor. */
    int serverPort;             /**< Porta do servidor. */
    char serverHostName[256];   /**< Nome do host do servidor. */
    int clientID;               /**< ID único do cliente. */
    char logPath[256];          /**< Caminho para o ficheiro de log. */
    bool isManual;              /**< Define se o jogo será jogado em modo manual. */
    int difficulty;             /**< Nível de dificuldade do jogo (ex.: 1 - fácil, 2 - médio, 3 - difícil). */
} clientConfig;

// Gera um ID único para o cliente.
int generateClientID();

// Carrega as configurações do cliente a partir de um ficheiro de configuração.
clientConfig getClientConfig(char *configPath);

#endif // CONFIG_H