#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "jogos.h"

void inicializaSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig config);
void enviarTabuleiro(int socket, Jogo *jogo);
int receberLinhaDoCliente(int socket, char *buffer);
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
        if ((newSockfd = accept(sockfd, (struct sockaddr *) 0, (int *) 0)) < 0)
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

void inicializaSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig config) {
    // Criar socket
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_dump(config.logPath, 0, 0, " : can't open stream socket", EVENT_CONNECTION_SERVER_ERROR);
    }

    // Limpar a estrutura do socket
    memset((char *) serv_addr, 0, sizeof(*serv_addr));

    // Preencher a estrutura do socket
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr->sin_port = htons(config.serverPort);

    // Associar o socket a um endereco qualquer
    if (bind(*sockfd, (struct sockaddr *) serv_addr, sizeof(*serv_addr)) < 0) {
        err_dump(config.logPath, 0, 0, " : can't bind local address", EVENT_CONNECTION_SERVER_ERROR);
    }

    // Ouvir o socket
    listen(*sockfd, 1);
}

void enviarTabuleiro(int socket, Jogo *jogo) {
    // Enviar tabuleiro ao cliente em formato JSON
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, "id", jogo->id);
    JSON_Value *tabuleiro_value = json_value_init_array();
    JSON_Array *tabuleiro_array = json_value_get_array(tabuleiro_value);

    for (int i = 0; i < 9; i++) {
        JSON_Value *linha_value = json_value_init_array();
        JSON_Array *linha_array = json_value_get_array(linha_value);
        for (int j = 0; j < 9; j++) {
            json_array_append_number(linha_array, jogo->tabuleiro[i][j]);
        }
        json_array_append_value(tabuleiro_array, linha_value);
    }

    json_object_set_value(root_object, "tabuleiro", tabuleiro_value);
    char *serialized_string = json_serialize_to_string(root_value);
    send(socket, serialized_string, strlen(serialized_string), 0);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

int receberLinhaDoCliente(int socket, char *buffer) {
    return recv(socket, buffer, 9, 0);
}

int validarLinha(char *buffer) {
    if (strlen(buffer) != 9) {
        return -1;
    }
    for (int i = 0; i < 9; i++) {
        if (buffer[i] < '0' || buffer[i] > '9') {
            return -1;
        }
    }
    return 0;
}
