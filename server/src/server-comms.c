#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs.h"
#include "server-comms.h"

void initializeSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig config) {

    // Criar socket
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_dump(config.logPath, 0, 0, "can't open stream socket", EVENT_CONNECTION_SERVER_ERROR);
    }

    // Limpar a estrutura do socket
    memset((char *) serv_addr, 0, sizeof(*serv_addr));

    // Preencher a estrutura do socket
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr->sin_port = htons(config.serverPort);

    // Associar o socket a um endereco qualquer
    if (bind(*sockfd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        err_dump(config.logPath, 0, 0, "can't bind local address", EVENT_CONNECTION_SERVER_ERROR);
    }

    // Ouvir o socket
    listen(*sockfd, 1);
}

void sendBoard(int *socket, Game *game) {
    // Enviar board ao cliente em formato JSON
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, "id", game->id);
    JSON_Value *board_value = json_value_init_array();
    JSON_Array *board_array = json_value_get_array(board_value);

    for (int i = 0; i < 9; i++) {
        JSON_Value *linha_value = json_value_init_array();
        JSON_Array *linha_array = json_value_get_array(linha_value);
        for (int j = 0; j < 9; j++) {
            json_array_append_number(linha_array, game->board[i][j]);
        }
        json_array_append_value(board_array, linha_value);
    }

    json_object_set_value(root_object, "board", board_value);
    char *serialized_string = json_serialize_to_string(root_value);
    send(*socket, serialized_string, strlen(serialized_string), 0);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

void receiveLines(int *newSockfd, Game *game, int playerID, ServerConfig config) {

    char buffer[256];

    // Receber e validar as linhas do cliente
    for (int i = 0; i < 9; i++) {
        int correctLine = 0;

        while (!correctLine) {
            // Limpar buffer
            memset(buffer, 0, sizeof(buffer));

            // Receber linha do cliente
            if (receiveLine(newSockfd, buffer) <= 0) {
                closeClientConnection(newSockfd, config.logPath, game->id, playerID, EVENT_CONNECTION_CLIENT_CLOSED);
                return;
            }

            // Converte a linha recebida em valores inteiros
            int insertLine[9];
            for (int j = 0; j < 9; j++) {
                insertLine[j] = buffer[j] - '0';
            }

            // Verificar a linha recebida com a função verifyLine
            correctLine = verifyLine(config.logPath, buffer, game, insertLine, i, playerID);

            // **Adicionei print para verificar a linha recebida**
            printf("Linha recebida do cliente: %s\n", buffer); 

            printf("Verificando linha %d...\n", i + 1);

            // Enviar resposta ao cliente
            if (correctLine) {
                strcpy(buffer, "correto");
            } else {
                strcpy(buffer, "incorreto: Valores incorretos na linha, tente novamente");
            }

            send(*newSockfd, buffer, strlen(buffer), 0);
            sendBoard(newSockfd, game);
        }
    }
}

int receiveLine(int *socket, char *buffer) {
    return recv(*socket, buffer, 9, 0);
}

void closeClientConnection(int *socket, char *logPath, int gameID, int playerID,  char *event) {

    // fechar a ligação com o cliente
    close(*socket);

    // escrever na consola a mensagem
    printf("Conexao terminada com o cliente %d e o jogo %d\n", playerID, gameID);

    // escrever no log a mensagem 
    writeLogJSON(logPath, gameID, playerID, event);
}