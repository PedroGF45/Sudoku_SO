#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs.h"
#include "client-comms.h"

void connectToServer(struct sockaddr_in *serv_addr, int *socketfd, clientConfig config) {
    /* Primeiro uma limpeza preventiva! memset é mais eficiente que bzero 
	   Dados para o socket stream: tipo */

    //memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr->sin_family = AF_INET; // endereços iternet DARPA

    /* Converter serverIP para binario*/
    if (inet_pton(AF_INET, config.serverIP, &serv_addr->sin_addr) <= 0) {

        // erro ao converter serverIP para binario
        err_dump(config.logPath, 0, config.clientID, " : can't get server address", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }

    /* Dados para o socket stream: porta do servidor */ 
    serv_addr->sin_port = htons(config.serverPort);

	/* Cria socket tcp (stream) */
    if ((*socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        // erro ao abrir socket
        err_dump(config.logPath, 0, config.clientID, " : can't open datagram socket", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }
    
	/* Estabelece ligação com o servidor */
    if (connect(*socketfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        // erro ao conectar ao servidor
        err_dump(config.logPath, 0, config.clientID, " : can't connect to server", EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED);
    }
    
    /* Print conexao estabelecida */
    printf("Conexao estabelecida com o servidor %s:%d\n", config.serverIP, config.serverPort);
    writeLogJSON(config.logPath, 0, config.clientID, EVENT_CONNECTION_CLIENT_ESTABLISHED);


}

void showMenu(int *socketfd, clientConfig config) {

    // print the menu
    printf(INTERFACE_MENU);

    // get the option from the user
    int option;
    scanf("%d", &option);

    // switch the option
    switch (option) {
        case 1:
            showPlayMenu(socketfd, config);
            break;
        case 2:
            //showStatisticsMenu();
            break;
        case 3:
            printf("Exiting...\n");
            exit(0);
            break;
        default:
            printf("Invalid option\n");
            break;
    }
}

void showPlayMenu(int *socketfd, clientConfig config) {

    // print the play menu
    printf(INTERFACE_PLAY_MENU);

    // get the option from the user
    int option;
    scanf("%d", &option);

    // switch the option
    switch (option) {
        case 1:
            showNewGameMenu(socketfd, config);
            break;
        case 2:
            //checkExistingGames();
            break;
        case 3:
            showMenu(socketfd, config);
            break;
        case 4:
            printf("Exiting...\n");
            exit(0);
            break;
        default:
            printf("Invalid option\n");
            break;
    }
}

void showNewGameMenu(int *socketfd, clientConfig config) {
    
    // ask for choosing a random game or a specific game
    printf(INTERFACE_SELECT_GAME);

    // get the option from the user
    int option;
    scanf("%d", &option);

    // switch the option
    switch (option) {
        case 1:
            // choose a random game
            playRandomGame(socketfd, config);
            break;
        case 2:
            //showGames();
            break;
        case 3:
            showPlayMenu(socketfd, config);
            break;
        case 4:
            printf("Exiting...\n");
            exit(0);
            break;
        default:
            printf("Invalid option\n");
            break;
    }
}

void playRandomGame(int *socketfd, clientConfig config) {

    // ask server for a random game
    if (send(*socketfd, "randomGame", strlen("randomGame"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, ": can't send random game request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting a random game...\n");
    }

}

void showBoard(char *buffer, char * logFileName, int gameID, int playerID) {

    // get the JSON object from the buffer
    JSON_Value *root_value = json_parse_string(buffer);
    JSON_Object *root_object = json_value_get_object(root_value);

    printf("-------------------------------------\n");

    // get the tabuleiro array from the JSON object
    JSON_Array *tabuleiro_array = json_object_get_array(root_object, "tabuleiro");

    for (int i = 0; i < json_array_get_count(tabuleiro_array); i++) {

        // get the line array from the tabuleiro array
        JSON_Array *linha_array = json_array_get_array(tabuleiro_array, i);

        printf("| line %d -> | ", i + 1);

        // print the line array
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

    // free the JSON object
    json_value_free(root_value);

    writeLogJSON(logFileName, gameID, playerID, EVENT_BOARD_SHOW);
}
