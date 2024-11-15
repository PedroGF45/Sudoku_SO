#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "../../utils/logs/logs.h"
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "server-comms.h"
#include "server-game.h"


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
 *   - Aceita uma ligação do cliente e cria uma estrutura `ClientData` que armazena as 
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuracao!\n");
        return 1;
    }

    // Carrega a configuracao do servidor
    ServerConfig *config = getServerConfig(argv[1]);

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
            err_dump(config->logPath, 0, 0, "accept error", EVENT_CONNECTION_SERVER_ERROR);
        } else {

            // elements to pass to thread: config, playerID, newSockfd
            ClientData *clientData = (ClientData *) malloc(sizeof(ClientData));
            if (clientData == NULL) {
                // erro ao alocar memoria
                err_dump(config->logPath, 0, 0, "can't allocate memory", MEMORY_ERROR);
                close(newSockfd);
                continue;
            }
            clientData->config = config;
            clientData->socket_fd = newSockfd;


            pthread_t thread;
            if (pthread_create(&thread, NULL, handleClient, (void *)clientData) != 0) {
                // erro ao criar thread
                err_dump(config->logPath, 0, 0, "can't create thread", EVENT_SERVER_THREAD_ERROR);
                free(clientData);
                close(newSockfd);
                continue;
            }
            
            // detach the thread
            pthread_detach(thread);
        }
    }

    close(sockfd);
    free(config);
    return 0;
}
