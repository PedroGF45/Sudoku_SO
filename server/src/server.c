#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "../../utils/logs/logs-common.h"
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "server-comms.h"
#include "server-game.h"
#include "../logs/logs.h"


/**
 * Função principal que configura e inicia o servidor, aceita conexões de clientes, 
 * e cria threads para gerir essas conexões.
 *
 * @param argc O número de argumentos passados na linha de comando.
 * @param argv Um array de strings que contém os argumentos da linha de comando. 
 * O primeiro argumento (argv[1]) deve ser o caminho para o ficheiro de configuração do servidor.
 * @return Retorna 0 se o servidor terminar normalmente ou 1 se faltar o argumento de configuração.
 *
 * @details Esta função realiza as seguintes operações:
 * - Verifica se o argumento de configuração foi fornecido. 
 * Se não for, imprime uma mensagem de erro e termina o programa.
 * - Carrega as configurações do servidor a partir do ficheiro de configuração especificado.
 * - Inicializa o socket do servidor e configura-o para aceitar conexões de clientes.
 * - Entra num loop infinito para aceitar conexões:
 *   - Aceita uma ligação do cliente e cria uma estrutura `Client` que armazena as 
 * informações necessárias para a nova conexão.
 *   - Cria uma nova thread para gerir cada cliente, usando a função `handleClient` para 
 * processar a comunicação com o cliente.
 *   - Detacha a thread para que possa ser gerida automaticamente pelo sistema operativo, 
 * sem necessidade de junção manual.
 * - Se houver um erro ao aceitar uma conexão ou criar uma thread, a função regista o erro 
 * no ficheiro de log especificado na configuração.
 *
 * @note O loop principal executa indefinidamente até que o servidor seja manualmente interrompido. 
 * O socket principal é fechado no final.
 */

ServerConfig* svConfig;

void handleSigInt(int sig) {
    printf("Server shutting down...\n");
    free(svConfig->clients);
    free(svConfig->rooms);
    free(svConfig);

    exit(0);
}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuracao!\n");
        return 1;
    }

    
    //signal(SIGINT, handleSigInt);

    printf("Server starting...\n");


    // Carrega a configuracao do servidor
    svConfig = getServerConfig(argv[1]);

    // Inicializa variáveis para socket
    int sockfd, newSockfd;
    struct sockaddr_in serv_addr;

    // Inicializar o socket
    initializeSocket(&serv_addr, &sockfd, svConfig);

    // Aguardar por conexões indefinidamente
    for (;;) {

        // Aceitar ligação
        if ((newSockfd = accept(sockfd, (struct sockaddr *) 0, 0)) < 0) {
            // erro ao aceitar ligacao
            err_dump(svConfig, 0, 0, "accept error", EVENT_CONNECTION_SERVER_ERROR);
        } else {

            // elements to pass to thread: config, playerID, newSockfd
            Client *client = (Client *) malloc(sizeof(Client));
            if (client == NULL) {
                // erro ao alocar memoria
                err_dump(svConfig, 0, 0, "can't allocate memory", MEMORY_ERROR);
                close(newSockfd);
                continue;
            }

            client->socket_fd = newSockfd;
            addClient(svConfig, client);

            client_data *data = (client_data *) malloc(sizeof(client_data));
            if (data == NULL) {
                // erro ao alocar memoria
                err_dump(svConfig, 0, 0, "can't allocate memory", MEMORY_ERROR);
                removeClient(svConfig, client);
                free(client);
                close(newSockfd);
                continue;
            }

            data->config = svConfig;
            data->client = client;

            // create a new thread to handle the client
            pthread_t thread;
            if (pthread_create(&thread, NULL, handleClient, (void *)data) != 0) {
                // erro ao criar thread
                err_dump(svConfig, 0, 0, "can't create client thread", EVENT_SERVER_THREAD_ERROR);
                removeClient(svConfig, client);
                free(client);
                free(data);
                close(newSockfd);
                continue;
            }
            
            // detach the thread
            pthread_detach(thread);
        }

        // Create a thread to handle consume for logs
        pthread_t logThread;
        if (pthread_create(&logThread, NULL, consumeLog, (void *)svConfig) != 0) {
            // erro ao criar thread
            err_dump(svConfig, 0, 0, "can't create log consumer thread", EVENT_SERVER_THREAD_ERROR);
            continue;
        }
    }

    close(sockfd);
    free(svConfig);
    return 0;
}
