#include <stdio.h>
#include "../../utils/logs/logs.h"
#include "../../utils/network/network.h"
#include "../config/config.h"

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuração!\n");
        return 1;
    }

    // Carrega a configuracao do cliente
    clientConfig config = getClientConfig(argv[1]);

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

    /* Primeiro uma limpeza preventiva! memset é mais eficiente que bzero 
	   Dados para o socket stream: tipo */

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; // endereços iternet DARPA

    /* Converter serverIP para binario*/
    if (inet_pton(AF_INET, config.serverIP, &serv_addr.sin_addr) <= 0) {

        // erro ao converter serverIP para binario
        err_dump(config.logPath, 0, config.clientID, " : can't get server address", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }

    /* Dados para o socket stream: porta do servidor */ 
	serv_addr.sin_port = htons(config.serverPort);

	/* Cria socket tcp (stream) */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        // erro ao abrir socket
        err_dump(config.logPath, 0, config.clientID, " : can't open datagram socket", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }
		
	/* Estabelece ligação com o servidor */
	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        // erro ao conectar ao servidor
        err_dump(config.logPath, 0, config.clientID, " : can't connect to server", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }
		
    /* Print conexao estabelecida */
    printf("Conexao estabelecida com o servidor %s:%d\n", config.serverIP, config.serverPort);
    writeLogJSON(config.logPath, 0, config.clientID, EVENT_CONNECTION_CLIENT_ESTABLISHED);

    // Pedir ao servidor para imprimir o menu
    char buffer[256];
    char logMessage[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    memset(logMessage, 0, sizeof(logMessage));
    strcpy(buffer, "menu");

    if ((send(sockfd, buffer, strlen(buffer), 0)) < 0) {   
        // erro ao enviar mensagem para o servidor
        err_dump(config.logPath, 0, config.clientID, " : can't send to server - menu", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        // mensagem enviada para o servidor
        snprintf(logMessage, sizeof(logMessage), "%s: %s", EVENT_MESSAGE_CLIENT_SENT, buffer);
        writeLogJSON(config.logPath, 0, config.clientID, logMessage);
    }

    // Receber mensagem do servidor
    memset(buffer, 0, sizeof(buffer));
    memset(logMessage, 0, sizeof(logMessage));

    if ((recv(sockfd, buffer, sizeof(buffer), 0)) < 0) {
        // erro ao receber mensagem do servidor
        err_dump(config.logPath, 0, config.clientID, "client: can't receive from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
    } else {

        // mensagem recebida do servidor
        snprintf(logMessage, sizeof(logMessage), "%s: %s", EVENT_MESSAGE_CLIENT_RECEIVED, buffer);
        writeLogJSON(config.logPath, 0, config.clientID, logMessage);

        // Imprimir mensagem recebida
        printf("Mensagem recebida do servidor:\n%s\n", buffer);
    }

    /* Fechar o socket */
	close (sockfd);
	exit(0);
}