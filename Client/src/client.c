#include <stdio.h>
#include "../../utils/logs/logs.h"
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "../../utils/parson/parson.h"

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuracao!\n");
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

    // Pedir ao servidor para imprimir o tabuleiro
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "client: can't receive tabuleiro from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
    } else {
        // Exibir o tabuleiro
        JSON_Value *root_value = json_parse_string(buffer);
        JSON_Object *root_object = json_value_get_object(root_value);
        printf("-------------------------------------\n");
        JSON_Array *tabuleiro_array = json_object_get_array(root_object, "tabuleiro");
        for (int i = 0; i < json_array_get_count(tabuleiro_array); i++) {
            JSON_Array *linha_array = json_array_get_array(tabuleiro_array, i);
            printf("| line %d -> | ", i + 1);
            for (int j = 0; j < 9; j++) {
                printf("%d ", (int)json_array_get_number(linha_array, j));
                if ((j + 1) % 3 == 0) {
                    printf("| ");
                }
            }
            printf("\n");
            if ((i + 1) % 3 == 0) {
                printf("-------------------------------------\n");
            }
        }
        json_value_free(root_value);
    }

    // Enviar linhas inseridas pelo utilizador e receber o tabuleiro atualizado
    for (int i = 0; i < 9; i++) {
        int linhaValida = 0;  // Variável para controlar se a linha está correta
        while (!linhaValida) {
            printf("Insira valores para a linha %d do tabuleiro (exactamente 9 digitos):\n", i + 1);
            scanf("%s", buffer);

            // Enviar a linha ao servidor
            send(sockfd, buffer, strlen(buffer), 0);

            // **Forçar a recepção do resultado do servidor antes de continuar**
            memset(buffer, 0, sizeof(buffer));
            if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
                err_dump(config.logPath, 0, config.clientID, "client: can't receive validation from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
                continue;
            }

            // Verificar se a linha foi validada corretamente
            if (strcmp(buffer, "correto") == 0) {
                printf("Linha %d está correta!\n", i + 1);
                linhaValida = 1;  // A linha está correta, pode avançar para a próxima
            } else {
                printf("Linha %d contém erros, tente novamente.\n", i + 1);
            }

            // Receber o tabuleiro atualizado
            memset(buffer, 0, sizeof(buffer));
            if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
                err_dump(config.logPath, 0, config.clientID, "client: can't receive updated tabuleiro from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
                continue;
            }

            // Exibir o tabuleiro atualizado
            JSON_Value *root_value = json_parse_string(buffer);
            JSON_Object *root_object = json_value_get_object(root_value);
            printf("-------------------------------------\n");
            JSON_Array *tabuleiro_array = json_object_get_array(root_object, "tabuleiro");
            for (int i = 0; i < json_array_get_count(tabuleiro_array); i++) {
                JSON_Array *linha_array = json_array_get_array(tabuleiro_array, i);
                printf("| line %d -> | ", i + 1);
                for (int j = 0; j < 9; j++) {
                    printf("%d ", (int)json_array_get_number(linha_array, j));
                    if ((j + 1) % 3 == 0) {
                        printf("| ");
                    }
                }
                printf("\n");
                if ((i + 1) % 3 == 0) {
                    printf("-------------------------------------\n");
                }
            }
            json_value_free(root_value);
        }
    }

    // Fechar o socket
    close(sockfd);
    exit(0);
}
