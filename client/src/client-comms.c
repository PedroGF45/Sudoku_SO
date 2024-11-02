#include <unistd.h>
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs.h"
#include "client-comms.h"

void connectToServer(struct sockaddr_in *serv_addr, int *socketfd, clientConfig config) {
    /* Primeiro uma limpeza preventiva! memset é mais eficiente que bzero 
	   Dados para o socket stream: tipo */

    memset(serv_addr, 0, sizeof(*serv_addr));
	serv_addr->sin_family = AF_INET; // endereços iternet DARPA

    /* Converter serverIP para binario*/
    if (inet_pton(AF_INET, config.serverIP, &serv_addr->sin_addr) <= 0) {
        // erro ao converter serverIP para binario
        err_dump(config.logPath, 0, config.clientID, "can't get server address", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }

    /* Dados para o socket stream: porta do servidor */ 
    serv_addr->sin_port = htons(config.serverPort);

	/* Cria socket tcp (stream) */
    if ((*socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        // erro ao abrir socket
        err_dump(config.logPath, 0, config.clientID, "can't open datagram socket", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }
    
	/* Estabelece ligação com o servidor */
    if (connect(*socketfd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        // erro ao conectar ao servidor
        err_dump(config.logPath, 0, config.clientID, "can't connect to server", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }
    
    /* Print conexao estabelecida */
    printf("Conexao estabelecida com o servidor %s:%d\n", config.serverIP, config.serverPort);
    writeLogJSON(config.logPath, 0, config.clientID, EVENT_CONNECTION_CLIENT_ESTABLISHED);


}

void showMenu(int *socketfd, clientConfig config) {

    int option;

    do {
        // Print the menu
        printf(INTERFACE_MENU);

        // Get the option from the user
        if (scanf("%d", &option) != 1) {
            printf("Invalid input. Please enter a number.\n");
            fflush(stdin); // Clear the input buffer
            continue;
        }

        // Process the option
        switch (option) {
            case 1:
                showPlayMenu(socketfd, config);
                break;
            case 2:
                // showStatisticsMenu();
                break;
            case 3:
                closeConnection(socketfd, config);
            default:
                printf("Invalid option. Please try again.\n");
        }
    } while (option < 1 || option > 3);
}

void showPlayMenu(int *socketfd, clientConfig config) {

    int option;

    do {
        // print the play menu
        printf(INTERFACE_PLAY_MENU);

        // Get the option from the user
        if (scanf("%d", &option) != 1) {
            printf("Invalid input. Please enter a number.\n");
            fflush(stdin); // Clear the input buffer
            continue;
        }

        // switch the option
        switch (option) {
            case 1:
                showSinglePLayerMenu(socketfd, config);
                break;
            case 2:
                showMultiPlayerMenu(socketfd, config);
                break;
            case 3:
                showMenu(socketfd, config);
                break;
            case 4:
                closeConnection(socketfd, config);
                break;
            default:
                printf("Invalid option\n");
                break;
        }
    } while (option < 1 || option > 4);
}

void showSinglePLayerMenu(int *socketfd, clientConfig config) {

    int option;
    
    do {
        // ask for choosing a random game or a specific game
        printf(INTERFACE_SELECT_GAME);

        // Get the option from the user
        if (scanf("%d", &option) != 1) {
            printf("Invalid input. Please enter a number.\n");
            fflush(stdin); // Clear the input buffer
            continue;
        }

        // switch the option
        switch (option) {
            case 1:
                // choose a random game
                playRandomSinglePlayerGame(socketfd, config);
                break;
            case 2:
                showGames(socketfd, config, true);
                break;
            case 3:
                showPlayMenu(socketfd, config);
                break;
            case 4:
                closeConnection(socketfd, config);
                break;
            default:
                printf("Invalid option\n");
                break;
        }
    } while (option < 1 || option > 4);
}

void playRandomSinglePlayerGame(int *socketfd, clientConfig config) {

    // ask server for a random game
    if (send(*socketfd, "newRandomSinglePLayerGame", strlen("newRandomSinglePLayerGame"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "can't send random game request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting a new random game...\n");

        // receive the board from the server
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

            // error receiving board from server
            err_dump(config.logPath, 0, config.clientID, "can't receive board from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

        } else {
            
            if (strcmp(buffer, "No rooms available") == 0) {
                printf("No rooms available\n");
                return;
            }

            // show the board
            showBoard(buffer, config.logPath, config.clientID);

        }
    }
}

void checkExistingGames(int *socketfd, clientConfig config) {
    // ask server for existing games
    if (send(*socketfd, "existingGames", strlen("existingGames"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "can't send existing games request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting existing games...\n");

        // receive the games from the server
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

            // error receiving games from server
            err_dump(config.logPath, 0, config.clientID, "can't receive games from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

        } else {

            // show the games
            printf("Existing games:\n%s\n", buffer);
            
        }
    }
}

void showMultiPlayerMenu(int *socketfd, clientConfig config) {
    
    int option;

    do {
        // ask for creating a room or joining a room
        printf(INTERFACE_SELECT_MULTIPLAYER_GAME);

        // Get the option from the user
        if (scanf("%d", &option) != 1) {
            printf("Invalid input. Please enter a number.\n");
            fflush(stdin); // Clear the input buffer
            continue;
        }

        // switch the option
        switch (option) {
            case 1:
                // create a room
                createNewMultiplayerGame(socketfd, config);
                break;
            case 2:
                // join a room
                showMultiplayerRooms(socketfd, config);
                break;
            case 3:
                showPlayMenu(socketfd, config);
                break;
            case 4:
                closeConnection(socketfd, config);
                break;
            default:
                printf("Invalid option\n");
                break;
        }
    } while (option < 1 || option > 4);
}

void createNewMultiplayerGame(int *socketfd, clientConfig config) {
    // create a new multiplayer game

    int option;

    do {
        // ask for creating a room or joining a room
        printf(INTERFACE_SELECT_GAME);

        // Get the option from the user
        if (scanf("%d", &option) != 1) {
            printf("Invalid input. Please enter a number.\n");
            fflush(stdin); // Clear the input buffer
            continue;
        }

        // switch the option
        switch (option) {
            case 1:
                // create a new random multiplayer game
                playRandomMultiPlayerGame(socketfd, config);
                break;
            case 2:
                // create a new specific multiplayer game
                showGames(socketfd, config, false);
                break;
            case 3:
                showMultiPlayerMenu(socketfd, config);
                break;
            case 4:
                closeConnection(socketfd, config);
                break;
            default:
                printf("Invalid option\n");
                break;
        }
    } while (option < 1 || option > 4);
}

// play a random multiplayer game
void playRandomMultiPlayerGame(int *socketfd, clientConfig config) {

    // ask server for a random game
    if (send(*socketfd, "newRandomMultiPlayerGame", strlen("newRandomMultiPlayerGame"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "can't send random multiplayer game request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting a new random multiplayer game...\n");

        // receive the board from the server
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

            // error receiving board from server
            err_dump(config.logPath, 0, config.clientID, "can't receive board from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

        } else {

            if (strcmp(buffer, "No rooms available") == 0) {
                printf("No rooms available\n");
                return;
            }
            
            // show the board
            showBoard(buffer, config.logPath, config.clientID);
        }
    }
}

void showMultiplayerRooms(int *socketfd, clientConfig config) {
    // ask server for existing rooms
    if (send(*socketfd, "existingRooms", strlen("existingRooms"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "can't send existing rooms request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting existing multiplayer rooms...\n");

        // receive the rooms from the server
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

            // error receiving rooms from server
            err_dump(config.logPath, 0, config.clientID, "can't receive rooms from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

        } else {

            // show the rooms
            printf("Existing rooms:\n%s\n", buffer);
            printf("0 - Back\n");

            // ask for the game ID
            int roomID;
            printf("Choose an option: ");

            // Get the game ID from the user
            if (scanf("%d", &roomID) != 1) {
                printf("Invalid input. Please enter a number.\n");
                fflush(stdin); // Clear the input buffer
                return;
            }

            if (roomID == 0) {

                // send 0 to the server
                if (send(*socketfd, "0", strlen("0"), 0) < 0) {
                    err_dump(config.logPath, 0, config.clientID, "can't send return to menu to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                } else {
                    char logMessage[256];
                    snprintf(logMessage, sizeof(logMessage), "%s: sent 0 to return to multiplayer menu", EVENT_MESSAGE_CLIENT_SENT);
                    writeLogJSON(config.logPath, 0, config.clientID, logMessage);
                }

                // show the single player menu
                showMultiPlayerMenu(socketfd, config);


            } else {

                // send the room ID to the server
                char roomIDString[10];
                sprintf(roomIDString, "%d", roomID);

                if (send(*socketfd, roomIDString, strlen(roomIDString), 0) < 0) {
                    err_dump(config.logPath, 0, config.clientID, "can't send room ID to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                } else {
                    printf("Requesting room with ID %s...\n", roomIDString);

                    // receive the board from the server
                    memset(buffer, 0, sizeof(buffer));

                    if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

                        // error receiving board from server
                        err_dump(config.logPath, 0, config.clientID, "can't receive board from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

                    } else {

                        // show the board
                        showBoard(buffer, config.logPath, config.clientID);
                    }
                }
            }
        }
    }
}

void sendLines(int *socketfd, clientConfig config) {

    // buffer for the board
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));

    // Enviar linhas inseridas pelo utilizador e receber o board atualizado
    for (int row = 0; row < 9; row++) {

        int validLine = 0;  // Variável para controlar se a linha está correta

        // line to send to server
        char line[10];

        // initiliaze buffer with '0' and null terminator
        memset(line, '0', sizeof(line));

        while (!validLine) {

            if (config.isManual) {

                printf("Insira valores para a linha %d do board (exactamente 9 digitos):\n", row + 1);
                scanf("%s", line);

            } else {

                resolveLine(buffer, line, row, config.difficulty);
            }

            printf("Linha envida: %s\n", line);

            // Enviar a linha ao servidor
            if (send(*socketfd, line, sizeof(line), 0) < 0) {
                err_dump(config.logPath, 0, config.clientID, "can't send lines to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                continue;
            }

            // Receber o board atualizado
            memset(buffer, 0, sizeof(buffer));
            if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {
                err_dump(config.logPath, 0, config.clientID, "can't receive updated board from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
                continue;
            }

            // Exibir o board atualizado
            JSON_Value *root_value = json_parse_string(buffer);
            JSON_Object *root_object = json_value_get_object(root_value);
            JSON_Array *board_array = json_object_get_array(root_object, "board");

            // check if the line is correct by comparing line and row of board
            JSON_Array *linha_array = json_array_get_array(board_array, row);
            for (int i = 0; i < 9; i++) {
                if (line[i] != (int)json_array_get_number(linha_array, i) + '0') {
                    validLine = 0;
                    break;
                } else {
                    validLine = 1;
                }
            }

            printf("-------------------------------------\n");
            for (int i = 0; i < json_array_get_count(board_array); i++) {
                JSON_Array *linha_array = json_array_get_array(board_array, i);
                printf("| line %d -> | ", i + 1);
                for (int j = 0; j < 9; j++) {
                    printf("%d ", (int)json_array_get_number(linha_array, j));
                    if ((j + 1) % 3 == 0) {
                        printf("| ");
                    }
                }
                printf("\n");
                if ((i + 1) % 3 == 0) {
                    printf("-------------------------------------\n");
                }
            }
            json_value_free(root_value);
        }
    }
}

void showGames(int *socketfd, clientConfig config, bool isSinglePlayer) {

    // message to send to the server
    char message[256];
    if (isSinglePlayer) {
        snprintf(message, sizeof(message), "selectSinglePlayerGames");
    } else {
        snprintf(message, sizeof(message), "selectMultiPlayerGames");
    }

    // ask server for existing games
    if (send(*socketfd, "selectSinglePlayerGames", strlen("selectSinglePlayerGames"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "can't send existing games request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting existing games...\n");

        // receive the games from the server
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

            // error receiving games from server
            err_dump(config.logPath, 0, config.clientID, "can't receive games from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

        } else {

            // show the games
            printf("Existing games:\n%s", buffer);
            printf("0 - Back\n");

            // ask for the game ID
            int gameID;
            printf("Choose an option: ");

            // Get the game ID from the user
            if (scanf("%d", &gameID) != 1) {
                printf("Invalid input. Please enter a number.\n");
                fflush(stdin); // Clear the input buffer
                return;
            }

            if (gameID == 0) {

                // send 0 to the server
                if (send(*socketfd, "0", strlen("0"), 0) < 0) {
                    err_dump(config.logPath, 0, config.clientID, "can't send return to menu to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                } else {
                    char logMessage[256];
                    snprintf(logMessage, sizeof(logMessage), "%s: sent 0 to return to menu", EVENT_MESSAGE_CLIENT_SENT);
                    writeLogJSON(config.logPath, 0, config.clientID, logMessage);
                }

                // show the single player menu
                if (isSinglePlayer) {
                    showSinglePLayerMenu(socketfd, config);
                } else {
                    showMultiPlayerMenu(socketfd, config);
                }

            } else {
                // send the game ID to the server
                char gameIDString[10];
                sprintf(gameIDString, "%d", gameID);

                if (send(*socketfd, gameIDString, strlen(gameIDString), 0) < 0) {
                    err_dump(config.logPath, 0, config.clientID, "can't send game ID to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                } else {
                    printf("Requesting game with ID %s...\n", gameIDString);

                    // receive the board from the server
                    memset(buffer, 0, sizeof(buffer));

                    if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

                        // error receiving board from server
                        err_dump(config.logPath, 0, config.clientID, "can't receive board from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

                    } else {

                        // show the board
                        showBoard(buffer, config.logPath, config.clientID);

                    }
                }
            }  
        }
    }
}

void closeConnection(int *socketfd, clientConfig config) {

    // send close connection message to the server
    if (send(*socketfd, "closeConnection", strlen("closeConnection"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "can't send close connection message to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Closing connection...\n");
        writeLogJSON(config.logPath, 0, config.clientID, EVENT_CONNECTION_CLIENT_CLOSED);
    }
}