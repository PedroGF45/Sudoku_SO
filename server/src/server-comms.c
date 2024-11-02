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

    int playerID = 0;
    Room *room;

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
    bool continueLoop = true;

    while (continueLoop) {

        bool startAgain = false;
        memset(buffer, 0, sizeof(buffer));

        // Receber menu status
        if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
            // erro ao receber modo de jogo
            err_dump(config->logPath, 0, playerID, "can't receive menu status", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
        } else {
            
            // cliente quer um novo jogo random
            if (strcmp(buffer, "newRandomSinglePLayerGame") == 0) {

                // criar novo jogo single player
                room = createRoomAndGame(&newSockfd, config, playerID, true, true, 0);

            // cliente quer random multiplayer game
            } else if (strcmp(buffer, "newRandomMultiPlayerGame") == 0) {

                // criar novo jogo single player
                room = createRoomAndGame(&newSockfd, config, playerID, false, true, 0);

            // cliente quer ver jogos existentes
            } else if (strcmp(buffer, "selectSinglePlayerGames") == 0 || strcmp(buffer, "selectMultiPlayerGames") == 0) {

                bool isSinglePlayer = strcmp(buffer, "selectSinglePlayerGames") == 0;
                
                // Receber jogos existentes
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));

                // Obter jogos existentes
                char *games = getGames(config);

                printf("Buffer: %s\n", buffer);
                printf("Jogos existentes: %s\n", games);

                bool leave = false;

                while (!leave) {

                    // Enviar jogos existentes ao cliente
                    if (send(newSockfd, games, strlen(games), 0) < 0) {

                        // erro ao enviar jogos existentes
                        err_dump(config->logPath, 0, playerID, "can't send existing games to client", EVENT_MESSAGE_SERVER_NOT_SENT);

                    } else {

                        // escrever no log
                        writeLogJSON(config->logPath, 0, playerID, EVENT_SERVER_GAMES_SENT);
                    }

                    printf("RECEBER IDS DOS JOGOS\n");
                    memset(buffer, 0, sizeof(buffer));

                    // receber ID do jogo
                    if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
                        // erro ao receber ID do jogo
                        err_dump(config->logPath, 0, playerID, "can't receive game ID from client", EVENT_MESSAGE_SERVER_NOT_RECEIVED);

                    // se o cliente mandar 0, voltar ao menu
                    } else if (atoi(buffer) == 0) {

                        printf("Cliente %d voltou atras no menu\n", playerID);
                        leave = true;
                        startAgain = true;

                    } else {

                        printf("Cliente %d escolheu o jogo single player com o ID: %s\n", playerID, buffer);

                        // obter ID do jogo
                        int gameID = atoi(buffer);

                        // carregar jogo
                        if (isSinglePlayer) {

                            // criar novo jogo single player
                            room = createRoomAndGame(&newSockfd, config, playerID, true, false, gameID);
                        } else {

                            // criar novo jogo multiplayer
                            room = createRoomAndGame(&newSockfd, config, playerID, false, false, gameID);
                        }

                        // sair do loop
                        leave = true;
                    }
                }

                // libertar memória alocada
                free(games);

            } else if (strcmp(buffer, "existingRooms") == 0) {

                // Receber jogos existentes
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));

                // Obter jogos existentes
                char *rooms = getRooms(config);

                printf("Buffer: %s\n", buffer);
                printf("Rooms existentes: %s\n", rooms);

                bool leave = false;

                while (!leave) {

                    // Enviar ids das rooms existentes ao cliente
                    if (send(newSockfd, rooms, strlen(rooms), 0) < 0) {

                        // erro ao enviar jogos existentes
                        err_dump(config->logPath, 0, playerID, "can't send existing games to client", EVENT_MESSAGE_SERVER_NOT_SENT);

                    } else {

                        // escrever no log
                        writeLogJSON(config->logPath, 0, playerID, EVENT_SERVER_GAMES_SENT);
                    }

                    printf("RECEBER IDS DAS ROOMS\n");
                    memset(buffer, 0, sizeof(buffer));

                    // receber ID do jogo
                    if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {

                        // erro ao receber ID do jogo
                        err_dump(config->logPath, 0, playerID, "can't receive room ID from client", EVENT_MESSAGE_SERVER_NOT_RECEIVED);

                    // se o cliente mandar 0, voltar ao menu
                    } else if (atoi(buffer) == 0) {

                        printf("Cliente %d voltou atras no menu\n", playerID);
                        leave = true;
                        startAgain = true;

                    } else {

                        printf("Cliente %d escolheu a sala multiplayer com o ID: %s\n", playerID, buffer);

                        // obter ID do jogo
                        int roomID = atoi(buffer);

                        // entrar na sala
                        room = joinRoom(config, roomID, playerID);
                        
                        // sair do loop
                        leave = true;
                    }
                }

                // libertar memória alocada
                printf("Rooms: %s\n", rooms);
                if (strcmp(rooms, "No rooms available") == 0) {
                    free(rooms);
                }
            } else if (strcmp(buffer, "closeConnection") == 0) {
                continueLoop = false;
                break;
            }

            if (!startAgain) {

                // Enviar tabuleiro ao cliente
                sendBoard(&newSockfd, room->game, config);

                // receber linhas do cliente
                receiveLines(&newSockfd, room->game, playerID, config);

                // terminar o jogo
                finishGame(&newSockfd, room, playerID, config);

            }
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

Room *createRoomAndGame(int *newSockfd, ServerConfig *config, int playerID, bool isSinglePlayer, bool isRandom, int gameID) {

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

    // load game
    Game *game;

    if (isRandom) {
        game = loadRandomGame(config, playerID);
    } else {
        game = loadGame(config, gameID, playerID);
    }

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
    room->numPlayers = 1;

    printf("New game created for client %d with game %d\n", playerID, room->game->id);

    return room;
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

void sendBoard(int *socket, Game *game, ServerConfig *config) {
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


    if (send(*socket, serialized_string, strlen(serialized_string), 0) < 0) {
        err_dump(config->logPath, game->id, 0, "can't send board to client", EVENT_MESSAGE_SERVER_NOT_SENT);
        return;
    }

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

void receiveLines(int *newSockfd, Game *game, int playerID, ServerConfig *config) {

    char buffer[BUFFER_SIZE];

    // Receber e validar as linhas do cliente
    for (int i = 0; i < 9; i++) {

        memset(buffer, 0, sizeof(buffer));

        int correctLine = 0;

        char line[10];

        while (!correctLine) {

            // Limpar linha
            memset(line, '0', sizeof(line));

            // Receber linha do cliente
            if (recv(*newSockfd, line, sizeof(line), 0) < 0) {
                err_dump(config->logPath, game->id, playerID, "can't receive line from client", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
                return;
            } else {

                printf("-----------------------------------------------------\n");
                // **Adicionei print para verificar a linha recebida**
                printf("Linha recebida do cliente %d: %s\n", playerID, line); 

                // Converte a linha recebida em valores inteiros
                int insertLine[9];
                for (int j = 0; j < 9; j++) {
                    insertLine[j] = line[j] - '0';
                }
                
                // Verificar a linha recebida com a função verifyLine
                correctLine = verifyLine(config->logPath, buffer, game, insertLine, i, playerID);

                if (correctLine == 1) {
                    // linha correta
                    printf("Linha %d correta\n", i + 1);
                } else {
                    // linha incorreta
                    printf("Linha %d incorreta\n", i + 1);
                }

                // wait for client to receive the line
                sleep(1);

                // Enviar o tabuleiro atualizado ao cliente
                sendBoard(newSockfd, game, config);
            }
        }
    }
}

void finishGame(int *socket, Room *room, int playerID, ServerConfig *config) {

    // Remove players from room
    for (int i = 0; i < room->maxPlayers; i++) {
        room->players[i] = 0;
    }

    // Reset number of players
    room->numPlayers = 0;
    room->maxPlayers = 0;
    
    // free game
    free(room->game);

    // free room
    free(room);

    // decrement number of rooms
    config->numRooms--;
}
