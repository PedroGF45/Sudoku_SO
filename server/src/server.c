#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/logs/logs.h"
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "server-comms.h"
#include "server-game.h"



int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuracao!\n");
        return 1;
    }

    // Carrega a configuracao do servidor
    ServerConfig config = getServerConfig(argv[1]);

    // Inicializa variáveis para socket
    int sockfd, newSockfd;
    struct sockaddr_in serv_addr;

    // Inicializar o socket
    initializeSocket(&serv_addr, &sockfd, config);

    // Aguardar por conexões indefinidamente
    for (;;) {

        // Aceitar ligação
        if ((newSockfd = accept(sockfd, (struct sockaddr *) 0, 0)) < 0) {
            // erro ao aceitar ligacao
            err_dump(config.logPath, 0, 0, "accept error", EVENT_CONNECTION_SERVER_ERROR);

        } else {
            
            // receber buffer do cliente
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));

            // Receber ID do jogador
            if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
                // erro ao receber ID do jogador
                err_dump(config.logPath, 0, 0, "can't receive player ID from client", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
            }

            // ID do jogador
            int playerID = atoi(buffer);

            printf("Conexao estabelecida com o cliente %d\n", playerID);
            writeLogJSON(config.logPath, 0, playerID, EVENT_CONNECTION_SERVER_ESTABLISHED);

            // Receber menu status
            if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
                // erro ao receber modo de jogo
                err_dump(config.logPath, 0, playerID, "can't receive menu status", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
            } else {
                
                if (strcmp(buffer, "randomGame") == 0) {

                    // Escolher um jogo aleatório
                    Game game = loadRandomGame(config, playerID);

                    // Enviar tabuleiro ao cliente
                    sendBoard(&newSockfd, &game);

                    // Receber linhas do cliente
                    receiveLines(&newSockfd, &game, playerID, config);
                } 
            }
        }

        // Fechar o socket do cliente
        close(newSockfd);
    }

    close(sockfd);
    return 0;
}
