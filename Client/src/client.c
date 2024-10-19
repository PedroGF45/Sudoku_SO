#include <stdio.h>
#include "../../utils/logs/logs.h"
#include "../config/config.h"

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuração!\n");
        return 1;
    }

    // Carrega a configuracao do cliente
    clientConfig config = getClientConfig(argv[1]);

    printf("IP do servidor: %s\n", config.serverIP);
    printf("Porta do servidor: %s\n", config.serverPort);
    printf("ID do cliente: %s\n", config.clientID);

    int idJogo = 29;
    int idJogador = 30;

    writeLogJSON(config.logPath, idJogo ,idJogador, EVENT_GAME_LOAD);
    writeLogJSON(config.logPath, idJogo ,idJogador, EVENT_BOARD_SHOW);
    writeLogJSON(config.logPath, idJogo ,idJogador, EVENT_SOLUTION_SENT);
    writeLogJSON(config.logPath, idJogo ,idJogador, EVENT_SOLUTION_CORRECT);
    writeLogJSON(config.logPath, idJogo ,idJogador, EVENT_SOLUTION_WRONG);
    
}