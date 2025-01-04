#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs-common.h"
#include "../logs/logs.h"
#include "client-comms.h"

/**
 * Estabelece uma ligação TCP ao servidor especificado na configuração do cliente.
 *
 * @param serv_addr Um pointer para a estrutura `sockaddr_in` que será configurada 
 * com o endereço e a porta do servidor.
 * @param socketfd Um pointer para o descritor de socket que será criado e usado para a conexão.
 * @param config A estrutura `clientConfig` que contém as informações de configuração do cliente, 
 * incluindo o IP e a porta do servidor.
 *
 * @details Esta função realiza os seguintes passos:
 * - Inicializa a estrutura `serv_addr` com `memset` para garantir que todos os campos 
 * estão corretamente definidos.
 * - Converte o endereço IP do servidor de formato textual para binário com `inet_pton`. 
 * Se falhar, regista o erro e termina.
 * - Configura a porta do servidor e cria um socket TCP (do tipo `SOCK_STREAM`). 
 * Se a criação do socket falhar, regista o erro.
 * - Tenta estabelecer uma ligação ao servidor com `connect`. Se a ligação falhar, regista o erro.
 * - Se a conexão for bem-sucedida, imprime uma mensagem de confirmação e regista o evento de 
 * ligação estabelecida no ficheiro de log.
 */

void connectToServer(struct sockaddr_in *serv_addr, int *socketfd, clientConfig *config) {
    /* Primeiro uma limpeza preventiva! memset é mais eficiente que bzero 
	   Dados para o socket stream: tipo */

    memset(serv_addr, 0, sizeof(*serv_addr));
	serv_addr->sin_family = AF_INET; // endereços iternet DARPA

    /* Converter serverIP para binario*/
    if (inet_pton(AF_INET, config->serverIP, &serv_addr->sin_addr) <= 0) {
        // erro ao converter serverIP para binario
        err_dump_client(config->logPath, 0, config->clientID, "can't get server address", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }

    /* Dados para o socket stream: porta do servidor */ 
    serv_addr->sin_port = htons(config->serverPort);

	/* Cria socket tcp (stream) */
    if ((*socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        // erro ao abrir socket
        err_dump_client(config->logPath, 0, config->clientID, "can't open datagram socket", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }
    
	/* Estabelece ligação com o servidor */
    if (connect(*socketfd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        // erro ao conectar ao servidor
        err_dump_client(config->logPath, 0, config->clientID, "can't connect to server", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }
    
    /* Print conexao estabelecida */
    printf("Conexao estabelecida com o servidor %s:%d\n", config->serverIP, config->serverPort);
    writeLogJSON(config->logPath, 0, config->clientID, EVENT_CONNECTION_CLIENT_ESTABLISHED);
}


/**
 * Envia uma mensagem ao servidor para fechar a conexão e regista o evento no log.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente, 
 * incluindo o caminho do log e o ID do cliente.
 *
 * @details A função realiza o seguinte:
 * - Envia uma mensagem ao servidor indicando que a conexão deve ser fechada.
 * - Se o envio falhar, regista o erro no ficheiro de log utilizando `err_dump_client`.
 * - Se o envio for bem-sucedido, imprime uma mensagem a indicar que a conexão está a ser encerrada e regista o evento de encerramento no log.
 */

void closeConnection(int *socketfd, clientConfig *config) {

    // send close connection message to the server
    if (send(*socketfd, "closeConnection", strlen("closeConnection"), 0) < 0) {
        err_dump_client(config->logPath, 0, config->clientID, "can't send close connection message to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Closing connection...\n");
        writeLogJSON(config->logPath, 0, config->clientID, EVENT_CONNECTION_CLIENT_CLOSED);
    }

    // close the socket
    close(*socketfd);

    // free config
    free(config);

    // exit the program
    exit(0);
}
