#include <stdlib.h>
#include <stdio.h>
#include "../../utils/logs/logs.h"
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "../../utils/parson/parson.h"
#include "client-comms.h"

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuracao!\n");
        return 1;
    }

    // Carrega a configuracao do cliente
    clientConfig config = getClientConfig(argv[1]);

    /* inicializa variaveis para socket
    socket descriptor
    sockaddr_in: estrutura para endereços de internet
    serv_addr: estrutura para endereços de internet
    */
    int sockfd;
    struct sockaddr_in serv_addr;

    // Conectar ao servidor
    connectToServer(&serv_addr, &sockfd, config);

    // Set buffer size and clear it
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // send client ID to server
    sprintf(buffer, "clientID");

    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        // erro ao enviar ID do cliente para o servidor
        err_dump(config.logPath, 0, 0, "can't ask for a client ID", EVENT_MESSAGE_CLIENT_NOT_SENT);

    } else {

        char logMessage[256];
        snprintf(logMessage, sizeof(logMessage), "%s: asked for a client ID", EVENT_MESSAGE_CLIENT_SENT);

        // log message sent to server
        writeLogJSON(config.logPath, 0, config.clientID, logMessage);
    }

    // receive client ID from server
    memset(buffer, 0, sizeof(buffer));
    if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
        // erro ao receber ID do cliente do servidor
        err_dump(config.logPath, 0, 0, "can't receive client ID", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
    } else {
        config.clientID = atoi(buffer);

        char logMessage[256];
        snprintf(logMessage, sizeof(logMessage), "%s: received client ID %d", EVENT_MESSAGE_CLIENT_RECEIVED, config.clientID);
        writeLogJSON(config.logPath, 0, config.clientID, logMessage);
    }

    bool continueLoop = true;

    while (continueLoop) {

        // Imprimir menu
        showMenu(&sockfd, config);

        // send lines to server
        sendLines(&sockfd, config);

        printf("Game finished!\n");
    }
    
    // Fechar o socket
    close(sockfd);
    exit(0);
}
