#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
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

            // elements to pass to thread: config, playerID, newSockfd
            ClientData *clientData = (ClientData *) malloc(sizeof(ClientData));
            clientData->config = config;
            clientData->socket_fd = newSockfd;


            pthread_t thread;
            if (pthread_create(&thread, NULL, handleClient, (void *)clientData) != 0) {
                // erro ao criar thread
                err_dump(config.logPath, 0, 0, "can't create thread", EVENT_SERVER_THREAD_ERROR);
            }
            pthread_detach(thread);
        }
    }

    close(sockfd);
    return 0;
}
