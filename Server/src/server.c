#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "jogos.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuracao!\n");
        return 1;
    }

    // Carrega a configuracao do servidor
    ServerConfig config = getServerConfig(argv[1]);

    // inicializa variaveis para socket
    int sockfd, newSockfd;
    struct sockaddr_in serv_addr;

    // Criar socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_dump(config.logPath, 0, 0, " : can't open stream socket", EVENT_CONNECTION_SERVER_ERROR);
    }

    // Limpar a estrutura do socket
    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    // Preencher a estrutura do socket
    serv_addr.sin_family = AF_INET; // enderecos de internet DARPA
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // aceitar ligacoes de qualquer endereco
    serv_addr.sin_port = htons(config.serverPort); // porta do servidor

    // Associar o socket a um endereco qualquer
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        err_dump(config.logPath, 0, 0, " : can't bind local address", EVENT_CONNECTION_SERVER_ERROR);
    }
		
    // Ouvir o socket
    listen(sockfd, 1);

    char buffer[256];
    char logMessage[BUFFER_SIZE];

    for (;;) {

        // Aceitar ligacao
        if ((newSockfd = accept(sockfd, (struct sockaddr *) 0, (int *) 0)) < 0)
            err_dump(config.logPath, 0, 0, " : accept error", EVENT_CONNECTION_SERVER_ERROR );

        // Print conexao estabelecida
        printf("Conexao estabelecida com um cliente\n");

        memset(buffer, 0, sizeof(buffer));
        memset(logMessage, 0, sizeof(logMessage));

        // Receber mensagem do cliente
        if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
            // Erro ao receber mensagem
            err_dump(config.logPath, 0, 0, " : read error", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
        }

        // Print mensagem recebida
        printf("Mensagem recebida do cliente:\n %s\n", buffer);

        // Escrever no log
        snprintf(logMessage, sizeof(logMessage), "%s: %s", EVENT_MESSAGE_SERVER_RECEIVED, buffer);
        writeLogJSON(config.logPath, 0, 0, logMessage);

        // se mensagem for "menu" envia o menu
        if (strcmp(buffer, "menu") == 0) {

            if (send(newSockfd, MENU_INTERFACE, strlen(MENU_INTERFACE), 0) < 0) {
                snprintf(logMessage, sizeof(logMessage), "%s: %s", EVENT_MESSAGE_SERVER_NOT_SENT, MENU_INTERFACE);
                err_dump(config.logPath, 0, 0, " : write error - menu", EVENT_MESSAGE_SERVER_NOT_SENT);
            }

            // Escrever no log
            snprintf(logMessage, sizeof(logMessage), "%s: %s", EVENT_MESSAGE_SERVER_SENT, MENU_INTERFACE);
            writeLogJSON(config.logPath, 0, 0, logMessage);
        }

        // Define idJogo e idJogador
        /*Aqui depois temos de definir valores incrementados para cada jogo e jogador
        ou um random para atribuir um tabuleiro aleatorio ao jogador*/
        int idJogo = 2;
        int idJogador = 2;

        // Load ao jogo
        Jogo jogo = carregaJogo(config, idJogo, idJogador);

        // Mostra o tabuleiro
        printf("Tabuleiro inicial:\n");
        mostraTabuleiro(config.logPath, jogo, idJogador);

        // Perguntar pela solução linha por linha
        int linhaInserida[9];
        // Armazena cada linha da solução como string
        char linha[10];  
        // Armazena os valores inseridos pelo utilizador
        char valoresInseridos[50];  

        for (int i = 0; i < 9; i++) {
            // Controla se a linha esta correta
            int linhaCorreta = 0;

            while (!linhaCorreta) {
                printf("Insira valores para a linha %d do tabuleiro (exactamente 9 digitos):\n", i + 1);
                // Ler a linha como uma string de no máximo 9 dígitos
                scanf("%9s", linha); 

                // Limpa a string de valores inseridos
                strcpy(valoresInseridos, "");  

                // Verifica o tamanho da entrada do utilizador
                int len = strlen(linha);
                if (len != 9) {
                    printf("A linha deve conter exatamente 9 digitos.\n");
                    // Pede novamente a linha se nao tiver 9 digitos
                    continue;
                }

                // Preenche a linhaInserida e prepara valoresInseridos para o log
                for (int j = 0; j < 9; j++) {
                    if (linha[j] >= '0' && linha[j] <= '9') {
                        // Converte char para int
                        linhaInserida[j] = linha[j] - '0';  
                    } else {
                        linhaInserida[j] = 0;  // Coloca 0 para caracteres não numéricos
                    }

                    // Adicionar o valor inserido a string
                    char valor[3];
                    // Preserva o caractere original
                    sprintf(valor, "%c", linha[j]);  
                    strcat(valoresInseridos, valor);
                    if (j < 8) strcat(valoresInseridos, " ");
                }

                // Verifica a linha inserida
                linhaCorreta = verificaLinha(config.logPath, valoresInseridos, &jogo, linhaInserida, i, idJogador);

                char logMessage[100];
                // Escreve no log se a linha está correta ou errada
                if (!linhaCorreta) {
                    printf("A linha %d contém erros ou esta incompleta.\n", i + 1);
                    sprintf(logMessage, "%s errada para a linha %d: %s", EVENT_SOLUTION_WRONG, i + 1, valoresInseridos);
                } else {
                    sprintf(logMessage, "%s para a linha %d: %s", EVENT_SOLUTION_CORRECT, i + 1, valoresInseridos);
                }
                writeLogJSON(config.logPath, idJogo, idJogador, logMessage);  // Log com a mensagem atualizada

                // Mostra o tabuleiro atualizado
                printf("Tabuleiro atualizado:\n");
                mostraTabuleiro(config.logPath, jogo, idJogador);
            }
        }

        // Fechar o socket
        close(newSockfd);
    }

    close(sockfd);
    return 0;
}
