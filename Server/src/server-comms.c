#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs.h"
#include "server-comms.h"

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

int receberLinhaDoCliente(int socket, char *buffer) {
    return recv(socket, buffer, 9, 0);
}
