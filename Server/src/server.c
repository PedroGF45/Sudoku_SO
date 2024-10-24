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

    // Inicializa variáveis para socket
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

        // Carregar o jogo do ficheiro JSON
        int idJogo = 2; // Pode ser parametrizado conforme necessidade
        int idJogador = 1; // Pode ser parametrizado conforme necessidade
        Jogo jogo = carregaJogo(config, idJogo, idJogador);

        // Enviar tabuleiro ao cliente em formato JSON
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        json_object_set_number(root_object, "id", jogo.id);
        JSON_Value *tabuleiro_value = json_value_init_array();
        JSON_Array *tabuleiro_array = json_value_get_array(tabuleiro_value);

        for (int i = 0; i < 9; i++) {
            JSON_Value *linha_value = json_value_init_array();
            JSON_Array *linha_array = json_value_get_array(linha_value);
            for (int j = 0; j < 9; j++) {
                json_array_append_number(linha_array, jogo.tabuleiro[i][j]);
            }
            json_array_append_value(tabuleiro_array, linha_value);
        }

        json_object_set_value(root_object, "tabuleiro", tabuleiro_value);
        char *serialized_string = json_serialize_to_string(root_value);
        send(newSockfd, serialized_string, strlen(serialized_string), 0);
        json_free_serialized_string(serialized_string);
        json_value_free(root_value);

        // Receber e validar as linhas do cliente
        for (int i = 0; i < 9; i++) {
            int linhaCorreta = 0;
            while (!linhaCorreta) {
                memset(buffer, 0, sizeof(buffer));

                // Receber linha do cliente
                int n = recv(newSockfd, buffer, sizeof(buffer), 0);
                if (n <= 0) {
                    printf("Conexao terminada com o cliente\n");
                    close(newSockfd);
                    break;
                }

                printf("Verificando linha %d...\n", i + 1);
                printf("Linha inserida: ");
                for (int j = 0; j < 9; j++) {
                    printf("%c ", buffer[j]);
                }
                printf("\n");

                // Verificar se a linha recebida tem exatamente 9 digitos
                if (strlen(buffer) != 9) {
                    strcpy(buffer, "incorreto: A linha deve ter exatamente 9 digitos");
                    send(newSockfd, buffer, strlen(buffer), 0);
                    // Enviar o tabuleiro mesmo se a linha estiver incorreta
                    goto enviar_tabuleiro;
                }

                // Converte a linha recebida em valores inteiros
                int linhaInserida[9];
                int inputValido = 1;
                for (int j = 0; j < 9; j++) {
                    if (buffer[j] < '0' || buffer[j] > '9') {
                        inputValido = 0;
                        break;
                    }
                    linhaInserida[j] = buffer[j] - '0';
                }

                // Se a linha contiver caracteres inválidos
                if (!inputValido) {
                    strcpy(buffer, "incorreto: A linha contem caracteres nao numericos");
                    send(newSockfd, buffer, strlen(buffer), 0);
                    // Enviar o tabuleiro mesmo se a linha estiver incorreta
                    goto enviar_tabuleiro;
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

                // Enviar o tabuleiro após a interação
                enviar_tabuleiro:

                // Enviar o tabuleiro atualizado ao cliente
                JSON_Value *updated_root_value = json_value_init_object();
                JSON_Object *updated_root_object = json_value_get_object(updated_root_value);
                json_object_set_number(updated_root_object, "id", jogo.id);
                JSON_Value *updated_tabuleiro_value = json_value_init_array();
                JSON_Array *updated_tabuleiro_array = json_value_get_array(updated_tabuleiro_value);

                for (int k = 0; k < 9; k++) {
                    JSON_Value *linha_value = json_value_init_array();
                    JSON_Array *linha_array = json_value_get_array(linha_value);
                    for (int j = 0; j < 9; j++) {
                        json_array_append_number(linha_array, jogo.tabuleiro[k][j]);
                    }
                    json_array_append_value(updated_tabuleiro_array, linha_value);
                }

                json_object_set_value(updated_root_object, "tabuleiro", updated_tabuleiro_value);
                char *updated_serialized_string = json_serialize_to_string(updated_root_value);
                send(newSockfd, updated_serialized_string, strlen(updated_serialized_string), 0);
                json_free_serialized_string(updated_serialized_string);
                json_value_free(updated_root_value);
            }
        }

        // Fechar o socket do cliente
        close(newSockfd);
    }

    close(sockfd);
    return 0;
}
