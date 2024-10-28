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

    for (;;) {
        // Aceitar ligação
        if ((newSockfd = accept(sockfd, (struct sockaddr *) 0, 0)) < 0) {
            // erro ao aceitar ligacao
            err_dump(config.logPath, 0, 0, " : accept error", EVENT_CONNECTION_SERVER_ERROR);

        } else {
            printf("Conexao estabelecida com um cliente\n");

            // Carregar o game do ficheiro JSON
            int idgame = 2; // Pode ser parametrizado conforme necessidade
            int playerID = 1; // Pode ser parametrizado conforme necessidade
            Game game = loadGame(config, idgame, playerID);

            // Enviar tabuleiro ao cliente
            sendBoard(&newSockfd, &game);

            receiveLines(&newSockfd, &game, playerID, config);
        }

        // Fechar o socket do cliente
        close(newSockfd);
    }

    close(sockfd);
    return 0;
}
