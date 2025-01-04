#include <stdio.h>
#include <string.h>
#include "../../utils/logs/logs-common.h"
#include "../logs/logs.h"
#include "client-menus.h"

/*
 * Exibe o menu principal do cliente e processa as opções selecionadas pelo utilizador.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente.
 *
 * @details A função faz o seguinte:
 * - Exibe o menu principal e solicita ao utilizador que escolha uma opção.
 * - Valida a entrada do utilizador para garantir que é um número.
 * - Processa a opção escolhida:
 *   - Opção 1: Chama a função `showPlayMenu` para exibir o menu de jogo.
 *   - Opção 2: Chama a função (comentada) para exibir as estatísticas (ainda por implementar).
 *   - Opção 3: Fecha a conexão com o servidor.
 * - Repete o loop até que o utilizador escolha uma opção válida (1 a 3).
 */

void showMenu(int *socketfd, clientConfig *config) {

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
                showStatisticsMenu(socketfd, config);
                showMenu(socketfd, config);
                break;
            case 3:
                closeConnection(socketfd, config);
            default:
                printf("Invalid option. Please try again.\n");
        }
    } while (option < 1 || option > 3);
}

/**
 * Exibe o menu de jogo e processa as opções selecionadas pelo utilizador.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente.
 *
 * @details A função faz o seguinte:
 * - Exibe o menu de opções de jogo e solicita ao utilizador que escolha uma opção.
 * - Valida a entrada do utilizador para garantir que é um número.
 * - Processa a opção escolhida:
 *   - Opção 1: Chama a função `showSinglePlayerMenu` para mostrar o menu de jogo single player.
 *   - Opção 2: Chama a função `showMultiPlayerMenu` para mostrar o menu de jogo multiplayer.
 *   - Opção 3: Retorna ao menu principal chamando `showMenu`.
 *   - Opção 4: Fecha a conexão com o servidor e termina o programa.
 * - Repete o loop até que o utilizador escolha uma opção válida (1 a 4).
 */

void showPlayMenu(int *socketfd, clientConfig *config) {

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


/**
 * Exibe o menu de jogo single player e processa as opções selecionadas pelo utilizador.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente.
 *
 * @details A função faz o seguinte:
 * - Exibe o menu de seleção de jogo single player e solicita ao utilizador que escolha uma opção.
 * - Valida a entrada do utilizador para garantir que é um número.
 * - Processa a opção escolhida:
 *   - Opção 1: Chama a função `playRandomSinglePlayerGame` para jogar um jogo aleatório.
 *   - Opção 2: Chama a função `showGames` para mostrar os jogos disponíveis.
 *   - Opção 3: Retorna ao menu de jogo chamando `showPlayMenu`.
 *   - Opção 4: Fecha a conexão com o servidor e termina o programa.
 * - Repete o loop até que o utilizador escolha uma opção válida (1 a 4).
 */

void showSinglePLayerMenu(int *socketfd, clientConfig *config) {

    int option;
    
    do {
        // ask for choosing a random game or a specific game
        printf(INTERFACE_SELECT_SINGLEPLAYER_GAME);

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
                playSinglePlayerGame(socketfd, config);
                break;
            case 2:
                // choose a specific single player game
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


/**
 * Joga um jogo aleatório single player.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente.
 *
 * @details A função faz o seguinte:
 * - Envia um pedido ao servidor para um novo jogo aleatório single player.
 * - Recebe o tabuleiro do servidor.
 * - Se o tabuleiro for "No rooms available", exibe uma mensagem e retorna.
 * - Chama a função `showBoard` para exibir o tabuleiro.
 */

void showStatisticsMenu(int *socketfd, clientConfig *client) {
    // Envia pedido de estatísticas ao servidor
    const char *request = "GET_STATS";
    if (send(*socketfd, request, strlen(request), 0) < 0) {
        // erro ao enviar pedido de estatísticas
        err_dump_client(client->logPath, 0, client->clientID, "can't send statistics request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Pedido de estatísticas enviado ao servidor\n");
        char logMessage[256];
        sprintf(logMessage, "%s: Pedido de estatísticas enviado ao servidor", EVENT_MESSAGE_CLIENT_SENT);
        writeLogJSON(client->logPath, 0, client->clientID, logMessage);
    }

    // Recebe e exibe as estatísticas do servidor
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {
        // erro ao receber estatísticas
        err_dump_client(client->logPath, 0, client->clientID, "can't receive statistics from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
    } else {
        printf("Estatísticas do servidor:\n%s\n", buffer);
    }
}

/**
 * Verifica se existem jogos disponíveis no servidor.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente.
 *
 * @details A função faz o seguinte:
 * - Envia um pedido ao servidor para obter os jogos disponíveis.
 * - Recebe a lista de jogos do servidor.
 * - Exibe a lista de jogos.
 */

/**
 * Exibe o menu de jogo multiplayer e processa as opções selecionadas pelo utilizador.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente.
 *
 * @details A função faz o seguinte:
 * - Exibe o menu de opções de jogo multiplayer e solicita ao utilizador que escolha uma opção.
 * - Valida a entrada do utilizador para garantir que é um número.
 * - Processa a opção escolhida:
 *   - Opção 1: Chama a função `createNewMultiplayerGame` para criar um novo jogo multiplayer.
 *   - Opção 2: Chama a função `showMultiplayerRooms` para mostrar as salas de jogo disponíveis.
 *   - Opção 3: Retorna ao menu de jogo chamando `showPlayMenu`.
 *   - Opção 4: Fecha a conexão com o servidor e termina o programa.
 * - Repete o loop até que o utilizador escolha uma opção válida (1 a 4).
 */

void showMultiPlayerMenu(int *socketfd, clientConfig *config) {
    
    int option;

    do {
        // ask for creating a room or joining a room
        printf(INTERFACE_SELECT_MULTIPLAYER_MENU);

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

void createNewMultiplayerGame(int *socketfd, clientConfig *config) {
    // create a new multiplayer game

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
                // create a new random multiplayer game
                showPossibleSynchronizations(socketfd, config);
                //playRandomMultiPlayerGame(socketfd, config);
                break;
            case 2:
                // show new specific multiplayer game
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


/**
 * Solicita ao servidor um jogo multiplayer aleatório e exibe o tabuleiro recebido.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente, 
 * incluindo o caminho do log e o ID do cliente.
 *
 * @details A função faz o seguinte:
 * - Envia um pedido ao servidor para iniciar um novo jogo multiplayer aleatório.
 * - Se o envio do pedido falhar, regista o erro no log e termina.
 * - Se o pedido for bem-sucedido, aguarda a receção do tabuleiro do jogo:
 *   - Se não houver salas disponíveis, informa o utilizador.
 *   - Se o tabuleiro for recebido corretamente, chama `showBoard` para exibir o tabuleiro.
 * - Regista quaisquer erros de comunicação (envio ou receção) no ficheiro de log.
 */

void showPossibleSynchronizations(int *socketfd, clientConfig *config) {

    int option;

    do {
        // ask for creating a room or joining a room
        printf(INTERFACE_POSSIBLE_SYNCHRONIZATION);

        // Get the option from the user
        if (scanf("%d", &option) != 1) {
            printf("Invalid input. Please enter a number.\n");
            fflush(stdin); // Clear the input buffer
            continue;
        }
        
        // switch the option
        switch (option) {
            case 1:
                // create a new random multiplayer game with readers-writers synchronization
                playMultiPlayerGame(socketfd, config, "readersWriters");
                writeLogJSON(config->logPath, 0, config->clientID, "Started multiplayer game with readers-writers synchronization");

                break;
            case 2:
                // create a new random multiplayer game with barber shop synchronization with priority queues
                playMultiPlayerGame(socketfd, config, "barberShopStaticPriority");
                writeLogJSON(config->logPath, 0, config->clientID, "Started multiplayer game with barberShopStaticPriority synchronization");
                break;
            case 3:
                // create a new random multiplayer game with barber shop synchronization with dynamic priority queues
                playMultiPlayerGame(socketfd, config, "barberShopDynamicPriority");
                writeLogJSON(config->logPath, 0, config->clientID, "Started multiplayer game with barberShopDynamicPriority synchronization");
                break;
            case 4:
                // create a new random multiplayer game with barber shop synchronization with a FIFO queue
                playMultiPlayerGame(socketfd, config, "barberShopFIFO");
                writeLogJSON(config->logPath, 0, config->clientID, "Started multiplayer game with barberShopFIFO synchronization");
                
                break;
            case 5:

                // send 0 to the server
                if (send(*socketfd, "0", strlen("0"), 0) < 0) {
                    err_dump_client(config->logPath, 0, config->clientID, "can't send return to menu to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                } else {
                    char logMessage[256];
                    snprintf(logMessage, sizeof(logMessage), "%s: sent 0 to return to multiplayer menu", EVENT_MESSAGE_CLIENT_SENT);
                    writeLogJSON(config->logPath, 0, config->clientID, logMessage);
                }

                // back to the multiplayer menu
                createNewMultiplayerGame(socketfd, config);
                writeLogJSON(config->logPath, 0, config->clientID, "Returned to multiplayer menu");
                break;
            case 6:
                // close the connection
                closeConnection(socketfd, config);
                break;
            default:
                printf("Invalid option\n");
                break;
        }
    } while (option < 1 || option > 6);

}

/**
 * Solicita ao servidor a lista de salas multiplayer existentes e permite ao utilizador 
 * escolher uma sala ou voltar atrás.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente, 
 * incluindo o caminho do log e o ID do cliente.
 *
 * @details A função faz o seguinte:
 * - Envia um pedido ao servidor para obter a lista de salas multiplayer disponíveis.
 * - Se o pedido falhar, regista o erro no log e termina.
 * - Se o pedido for bem-sucedido, recebe a lista de salas do servidor e exibe-a.
 * - Permite ao utilizador escolher uma sala pelo ID ou voltar ao menu anterior:
 *   - Se o utilizador escolher 0, envia o pedido de retorno ao servidor e exibe o menu multiplayer.
 *   - Se for escolhido um ID de sala, envia o ID ao servidor e aguarda o tabuleiro da sala.
 * - Regista todos os erros de comunicação e eventos importantes no ficheiro de log.
 */

void showMultiplayerRooms(int *socketfd, clientConfig *config) {
    // ask server for existing rooms
    if (send(*socketfd, "existingRooms", strlen("existingRooms"), 0) < 0) {
        err_dump_client(config->logPath, 0, config->clientID, "can't send existing rooms request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
        
    } else {
        printf("Requesting existing multiplayer rooms...\n");
        writeLogJSON(config->logPath, 0, config->clientID, "Sent existing rooms request to server");

        // receive the rooms from the server
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

            // error receiving rooms from server
            err_dump_client(config->logPath, 0, config->clientID, "can't receive rooms from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
        
        } else {

            // show the rooms
            printf("Existing rooms:\n%s\n", buffer);
            printf("0 - Back\n");
            writeLogJSON(config->logPath, 0, config->clientID, "Received existing rooms from server");

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
                    err_dump_client(config->logPath, 0, config->clientID, "can't send return to menu to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                } else {
                    char logMessage[256];
                    snprintf(logMessage, sizeof(logMessage), "%s: sent 0 to return to multiplayer menu", EVENT_MESSAGE_CLIENT_SENT);
                    writeLogJSON(config->logPath, 0, config->clientID, logMessage);
                }

                // show the multiplayer menu
                showMultiPlayerMenu(socketfd, config);


            } else {

                // send the room ID to the server
                char roomIDString[10];
                sprintf(roomIDString, "%d", roomID);

                if (send(*socketfd, roomIDString, strlen(roomIDString), 0) < 0) {
                    err_dump_client(config->logPath, 0, config->clientID, "can't send room ID to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                } else {
                    printf("Requesting room with ID %s...\n", roomIDString);
                }
                    
                //printf("NOW RECEIVING TIMER\n");
                // receive timer from server
                receiveTimer(socketfd, config);
            }
        }
    }
}

/**
 * Solicita ao servidor a lista de jogos existentes (single player ou multiplayer) e 
 * permite ao utilizador escolher um jogo ou voltar ao menu.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente, 
 * incluindo o caminho do log e o ID do cliente.
 * @param isSinglePlayer Um valor booleano que indica se o utilizador quer jogos 
 * single player (`true`) ou multiplayer (`false`).
 *
 * @details A função realiza as seguintes operações:
 * - Envia um pedido ao servidor para obter a lista de jogos disponíveis, 
 * com base no tipo de jogo (single player ou multiplayer).
 * - Se o pedido falhar, regista o erro no log e termina.
 * - Se o pedido for bem-sucedido, recebe e exibe a lista de jogos.
 * - Permite ao utilizador escolher um jogo pelo ID ou voltar ao menu anterior:
 *   - Se o utilizador escolher 0, envia a escolha ao servidor e exibe o menu apropriado (single player ou multiplayer).
 *   - Se for escolhido um ID de jogo, envia o ID ao servidor, recebe o tabuleiro e chama `showBoard` para o exibir.
 * - Regista quaisquer erros de comunicação e eventos importantes no ficheiro de log.
 */
void showGames(int *socketfd, clientConfig *config, bool isSinglePlayer) {

    // message to send to the server
    char message[256];
    if (isSinglePlayer) {
        snprintf(message, sizeof(message), "selectSinglePlayerGames");
    } else {
        snprintf(message, sizeof(message), "selectMultiPlayerGames");
    }

    // ask server for existing games
    if (send(*socketfd, message, strlen(message), 0) < 0) {
        err_dump_client(config->logPath, 0, config->clientID, "can't send existing games request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        writeLogJSON(config->logPath, 0, config->clientID, "Requested existing games from server");
        printf("Requesting existing games...\n");

        // receive the games from the server
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

            // error receiving games from server
            err_dump_client(config->logPath, 0, config->clientID, "can't receive games from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

        } else {

            // show the games
            printf("Existing games:\n%s", buffer);
            printf("0 - Back\n");
            writeLogJSON(config->logPath, 0, config->clientID, "Received and displayed existing games");
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
                    err_dump_client(config->logPath, 0, config->clientID, "can't send return to menu to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                } else {
                    char logMessage[256];
                    snprintf(logMessage, sizeof(logMessage), "%s: sent 0 to return to menu", EVENT_MESSAGE_CLIENT_SENT);
                    writeLogJSON(config->logPath, 0, config->clientID, logMessage);
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
                    err_dump_client(config->logPath, 0, config->clientID, "can't send game ID to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                } else {
                    printf("Requesting game with ID %s...\n", gameIDString);
                }

                if (!isSinglePlayer) {
                    // now need to choose synchronization
                    showPossibleSynchronizations(socketfd, config);
                    writeLogJSON(config->logPath, 0, config->clientID, "Selected synchronization type for multiplayer game");
                }
            }  
        }
    }
}

void receiveTimer(int *socketfd, clientConfig *config) {

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // receive at 60, 50, 40, 30, 20, 10, 5, 4, 3, 2, 1 seconds
    int timeLeft = 60;

    bool isRoomFull = false;

    //printf("IN RECEIVING TIMER\n");

    while (timeLeft > 0) {
        if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

            // error receiving timer from server
            err_dump_client(config->logPath, 0, config->clientID, "can't receive timer from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

        } else {

            //printf("Buffer: %s\n", buffer);

            // check if buffer is "Room is full"
            if (strcmp(buffer, "Room is full") == 0) {
                
                isRoomFull = true;
                char logMessage[256];
                snprintf(logMessage, sizeof(logMessage), "%s: room is full", EVENT_MESSAGE_CLIENT_RECEIVED);
                writeLogJSON(config->logPath, 0, config->clientID, logMessage);
                break;
            }

            timeLeft = showTimerUpdate(buffer, timeLeft);

            char logMessage[256];
            snprintf(logMessage, sizeof(logMessage), "%s: time left: %d", EVENT_MESSAGE_CLIENT_RECEIVED, timeLeft);
            writeLogJSON(config->logPath, 0, config->clientID, logMessage);
        }
    }

    if (isRoomFull) {
        //debug
        writeLogJSON(config->logPath, 0, config->clientID, "Room is full. Returning to multiplayer menu.");
        printf("Room is full\n");
        // show the multiplayer menu
        showMultiPlayerMenu(socketfd, config);
    }
}

void playSinglePlayerGame(int *socketfd, clientConfig *config) {

    // ask server for a random game
    if (send(*socketfd, "newSinglePlayerGame", strlen("newSinglePlayerGame"), 0) < 0) {
        err_dump_client(config->logPath, 0, config->clientID, "can't send game request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting a new game...\n");
    }
}

void playMultiPlayerGame(int *socketfd, clientConfig *config, char *synchronization) {

    // buffer
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // check for synchronization
    if (strcmp(synchronization, "readersWriters") == 0) {
        
        // buffer for readersWriters
        strcpy(buffer, "newMultiPlayerGameReadersWriters");
     // Log evento de solicitação
        writeLogJSON(config->logPath, 0, config->clientID, "Requesting multiplayer game with readers-writers synchronization");
    } else if (strcmp(synchronization, "barberShopStaticPriority") == 0) {
        
        // buffer for barberShopPriority
        strcpy(buffer, "newMultiPlayerGameBarberShopStaticPriority");
         // Log evento de solicitação
        writeLogJSON(config->logPath, 0, config->clientID, "Requesting multiplayer game with barber shop static priority synchronization");
    
    } else if (strcmp(synchronization, "barberShopDynamicPriority") == 0) {
        
        // buffer for barberShopDynamicPriority
        strcpy(buffer, "newMultiPlayerGameBarberShopDynamicPriority");
         // Log evento de solicitação
        writeLogJSON(config->logPath, 0, config->clientID, "Requesting multiplayer game with barber shop dynamic priority synchronization");

    } else if (strcmp(synchronization, "barberShopFIFO") == 0) {
        
        // buffer for barberShopFIFO
        strcpy(buffer, "newMultiPlayerGameBarberShopFIFO");
        writeLogJSON(config->logPath, 0, config->clientID, "Requesting multiplayer game with barber shop FIFO synchronization");

    } else {
        printf("Invalid synchronization option\n");
        writeLogJSON(config->logPath, 1, config->clientID, "Invalid synchronization option requested");
        
        return;
    }

    // ask server for a 
    if (send(*socketfd, buffer, BUFFER_SIZE, 0) < 0) {
        err_dump_client(config->logPath, 0, config->clientID, "can't send multiplayer game request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting a new multiplayer game...\n");

        // receive timer from server
        receiveTimer(socketfd, config);

        printf("JOGO INICIADO\n");
    }
}

int showTimerUpdate(char *buffer, int timeLeft) {

    //printf("Recebido: %s\n", buffer);
    // Parse the received message
    strtok(buffer, "\n");
    timeLeft = atoi(strtok(NULL, "\n"));
    //printf("Tempo restante: %d segundos\n", timeLeft);
    int roomId = atoi(strtok(NULL, "\n"));
    //printf("ID da sala: %d\n", roomId);
    int gameId = atoi(strtok(NULL, "\n"));
    //printf("ID do jogo: %d\n", gameId);
    int numPlayers = atoi(strtok(NULL, "\n"));
    //printf("Jogadores na sala: %d\n", numPlayers);

    // show the timer update
    printf("Time left: %d seconds - Room ID: %d - Game ID: %d - Players joined: %d\n", timeLeft, roomId, gameId, numPlayers);

    return --timeLeft;
}