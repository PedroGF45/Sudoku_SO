#include <stdlib.h>
#include <stdio.h>
#include <time.h>
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

    // Garante que os ids aleatórios são diferentes
    srand(time(NULL));

    // Carrega a configuracao do cliente
    clientConfig config = getClientConfig(argv[1]);

    // Se o ID do cliente for 0 gera um ID aleatório
    if (config.clientID == 0) {
        config.clientID = rand() % 1000; 
    }

    printf("IP do servidor: %s\n", config.serverIP);
    printf("Porta do servidor: %d\n", config.serverPort);
    printf("Hostname do servidor: %s\n", config.serverHostName);
    printf("ID do cliente: %d\n", config.clientID);
    printf("Log path do cliente: %s\n", config.logPath);

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
    sprintf(buffer, "%d", config.clientID);
    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        // erro ao enviar ID do cliente para o servidor
        err_dump(config.logPath, 0, config.clientID, "can't send client ID", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {

        char logMessage[256];
        sprintf(logMessage, "%s: Client ID %d", EVENT_MESSAGE_CLIENT_SENT, config.clientID);

        // log message sent to server
        writeLogJSON(config.logPath, 0, config.clientID, logMessage);
    }

    // Imprimir menu
    showMenu(&sockfd, config);

    // send lines to server
    sendLines(&sockfd, config);
    
    // Fechar o socket
    close(sockfd);
    exit(0);
}