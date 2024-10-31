#include <pthread.h>
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs.h"
#include "server-comms.h"


static int nextPlayerID = 1;

int generateUniqueClientId() {
    return nextPlayerID++;
}

void *handleClient(void *arg) {

    // Obter dados do cliente
    ClientData *clientData = (ClientData *) arg;
    ServerConfig *config = clientData->config;
    int newSockfd = clientData->socket_fd;

    // receber buffer do cliente
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    int playerID;

    // Receber ID do jogador
    if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {

        // erro ao receber ID do jogador
        err_dump(config->logPath, 0, 0, "can't receive question for client ID", EVENT_MESSAGE_SERVER_NOT_RECEIVED);

    } else if (strcmp(buffer, "clientID") == 0) {

        printf("Conexao estabelecida com um novo cliente\n");

        // enviar ID do jogador
        playerID = generateUniqueClientId();
        sprintf(buffer, "%d", playerID);

        if (send(newSockfd, buffer, strlen(buffer), 0) < 0) {

            // erro ao enviar ID do jogador
            err_dump(config->logPath, 0, 0, "can't send client ID", EVENT_MESSAGE_SERVER_NOT_SENT);
        }

        printf("ID atribuido ao novo cliente: %d\n", playerID);
        writeLogJSON(config->logPath, 0, playerID, EVENT_CONNECTION_SERVER_ESTABLISHED);
    }

    // Receber menu status
    if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
        // erro ao receber modo de jogo
        err_dump(config->logPath, 0, playerID, "can't receive menu status", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
    } else {
        
        // cliente quer um novo jogo random
        if (strcmp(buffer, "newRandomSinglePLayerGame") == 0) {

            // criar novo jogo single player
            Game *game = createRoomAndGame(&newSockfd, config, playerID, true);

            // Enviar tabuleiro ao cliente
            sendBoard(&newSockfd, game);

            // receber linhas do cliente
            receiveLines(&newSockfd, game, playerID, config);


        // cliente quer random multiplayer game
        } else if (strcmp(buffer, "newRandomMultiPlayerGame") == 0) {

            // criar novo jogo single player
            Game *game = createRoomAndGame(&newSockfd, config, playerID, false);

            // Enviar tabuleiro ao cliente
            sendBoard(&newSockfd, game);

            // receber linhas do cliente
            receiveLines(&newSockfd, game, playerID, config);

        // cliente quer ver jogos existentes
        } else if (strcmp(buffer, "existingGames") == 0) {
            
            // Receber jogos existentes
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));

            // Obter jogos existentes
            char *games = getExistingGames(config);

            // Enviar jogos existentes ao cliente
            send(newSockfd, games, strlen(games), 0);

            // escrever no log
            writeLogJSON(config->logPath, 0, playerID, EVENT_SERVER_GAMES_SENT);
        }
    }

    // fechar a ligação com o cliente
    close(newSockfd);
    // libertar a memória alocada
    free(clientData);
    printf("Conexao terminada com o cliente %d\n", playerID);
    writeLogJSON(config->logPath, 0, playerID, EVENT_SERVER_CONNECTION_FINISH );

    // terminar a thread
    pthread_exit(NULL);
}

Game *createRoomAndGame(int *newSockfd, ServerConfig *config, int playerID, bool isSinglePlayer) {

    // check if there's config->rooms is full by looping
    if (config->numRooms == config->maxRooms) {
        // send message to client
        send(*newSockfd, "No rooms available", strlen("No rooms available"), 0);
        // write log
        writeLogJSON(config->logPath, 0, playerID, "No rooms available");
        return NULL;
    }

    // create room
    Room *room = createRoom(config);

    // add room to config
    config->rooms[room->id - 1] = room;

    // load random game
    Game *game = loadRandomGame(config, playerID);

    if (game == NULL) {
        fprintf(stderr, "Error loading random game\n");
        exit(1);
    }

    // add game to room
    room->game = game;

    if (isSinglePlayer) {
        // set max players
        room->maxPlayers = 1;
    } else {
        // set max players
        room->maxPlayers = config->maxPlayersPerRoom;
    }

    // associate player to game
    room->players[0] = playerID;

    printf("New random multiplayer game created for client %d with game %d\n", playerID, room->game->id);

    return game;
}

void initializeSocket(struct sockaddr_in *serv_addr, int *sockfd, ServerConfig *config) {

    // Criar socket
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_dump(config->logPath, 0, 0, "can't open stream socket", EVENT_CONNECTION_SERVER_ERROR);
    }

    // Limpar a estrutura do socket
    memset((char *) serv_addr, 0, sizeof(*serv_addr));

    // Preencher a estrutura do socket
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr->sin_port = htons(config->serverPort);

    // Associar o socket a um endereco qualquer
    if (bind(*sockfd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        err_dump(config->logPath, 0, 0, "can't bind local address", EVENT_CONNECTION_SERVER_ERROR);
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

void receiveLines(int *newSockfd, Game *game, int playerID, ServerConfig *config) {

    char buffer[256];

    // Receber e validar as linhas do cliente
    for (int i = 0; i < 9; i++) {
        int correctLine = 0;

        while (!correctLine) {
            // Limpar buffer
            memset(buffer, 0, sizeof(buffer));

            // Receber linha do cliente
            if (receiveLine(newSockfd, buffer) <= 0) {
                closeClientConnection(newSockfd, config->logPath, game->id, playerID, EVENT_CONNECTION_CLIENT_CLOSED);
                return;
            }

            // Converte a linha recebida em valores inteiros
            int insertLine[9];
            for (int j = 0; j < 9; j++) {
                insertLine[j] = buffer[j] - '0';
            }

            printf("-----------------------------------------------------\n");

            // **Adicionei print para verificar a linha recebida**
            printf("Linha recebida do cliente %d: %s\n", playerID, buffer); 

            printf("Verificando linha %d...\n", i + 1);

            // Verificar a linha recebida com a função verifyLine
            correctLine = verifyLine(config->logPath, buffer, game, insertLine, i, playerID);

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