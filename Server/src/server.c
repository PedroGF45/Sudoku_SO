#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "server-comms.h"
#include "jogos.h"

int validarLinha(char *buffer);

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
    inicializaSocket(&serv_addr, &sockfd, config);

    char buffer[256];

    for (;;) {
        // Aceitar ligação
        if ((newSockfd = accept(sockfd, (struct sockaddr *) 0, 0)) < 0)
            err_dump(config.logPath, 0, 0, " : accept error", EVENT_CONNECTION_SERVER_ERROR);

        printf("Conexao estabelecida com um cliente\n");

        // Carregar o jogo do ficheiro JSON
        int idJogo = 2; // Pode ser parametrizado conforme necessidade
        int idJogador = 1; // Pode ser parametrizado conforme necessidade
        Jogo jogo = carregaJogo(config, idJogo, idJogador);

        // Enviar tabuleiro ao cliente
        enviarTabuleiro(newSockfd, &jogo);

        // Receber e validar as linhas do cliente
        for (int i = 0; i < 9; i++) {
            int linhaCorreta = 0;
            while (!linhaCorreta) {
                memset(buffer, 0, sizeof(buffer));

                // Receber linha do cliente
                if (receberLinhaDoCliente(newSockfd, buffer) <= 0) {
                    printf("Conexao terminada com o cliente\n");
                    close(newSockfd);
                    break;
                }

                // **Adicionei print para verificar a linha recebida**
                printf("Linha recebida do cliente: %s\n", buffer); 

                printf("Verificando linha %d...\n", i + 1);

                // Validar linha
                if (validarLinha(buffer) != 0) {
                    strcpy(buffer, "incorreto: A linha deve ter exatamente 9 digitos e ser numerica");
                    send(newSockfd, buffer, strlen(buffer), 0);
                    enviarTabuleiro(newSockfd, &jogo);
                    continue;
                }

                // Converte a linha recebida em valores inteiros
                int linhaInserida[9];
                for (int j = 0; j < 9; j++) {
                    linhaInserida[j] = buffer[j] - '0';
                }

                // Verificar a linha recebida com a função verificaLinha
                linhaCorreta = verificaLinha(config.logPath, buffer, &jogo, linhaInserida, i, idJogador);

                // Enviar resposta ao cliente
                if (linhaCorreta) {
                    strcpy(buffer, "correto");
                } else {
                    strcpy(buffer, "incorreto: Valores incorretos na linha, tente novamente");
                }
                send(newSockfd, buffer, strlen(buffer), 0);
                enviarTabuleiro(newSockfd, &jogo);
            }
        }

        // Fechar o socket do cliente
        close(newSockfd);
    }

    close(sockfd);
    return 0;
}
