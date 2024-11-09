#include <unistd.h>
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs.h"
#include "client-comms.h"



/**
 * Estabelece uma ligação TCP ao servidor especificado na configuração do cliente.
 *
 * @param serv_addr Um pointer para a estrutura `sockaddr_in` que será configurada 
 * com o endereço e a porta do servidor.
 * @param socketfd Um pointer para o descritor de socket que será criado e usado para a conexão.
 * @param config A estrutura `clientConfig` que contém as informações de configuração do cliente, 
 * incluindo o IP e a porta do servidor.
 *
 * @details Esta função realiza os seguintes passos:
 * - Inicializa a estrutura `serv_addr` com `memset` para garantir que todos os campos 
 * estão corretamente definidos.
 * - Converte o endereço IP do servidor de formato textual para binário com `inet_pton`. 
 * Se falhar, regista o erro e termina.
 * - Configura a porta do servidor e cria um socket TCP (do tipo `SOCK_STREAM`). 
 * Se a criação do socket falhar, regista o erro.
 * - Tenta estabelecer uma ligação ao servidor com `connect`. Se a ligação falhar, regista o erro.
 * - Se a conexão for bem-sucedida, imprime uma mensagem de confirmação e regista o evento de 
 * ligação estabelecida no ficheiro de log.
 */

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


/**
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

void playRandomSinglePlayerGame(int *socketfd, clientConfig config) {

    // ask server for a random game
    if (send(*socketfd, "newRandomSinglePLayerGame", strlen("newRandomSinglePLayerGame"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "can't send random game request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting a new random game...\n");
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


/**
 * Cria um novo jogo multiplayer, permitindo ao utilizador escolher entre criar uma nova sala ou entrar numa existente.
 *
 * @param socketfd Um ponteiro para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente.
 *
 * @details A função faz o seguinte:
 * - Exibe um menu que permite ao utilizador escolher entre as seguintes opções:
 *   - Opção 1: Cria um novo jogo multiplayer aleatório chamando `playRandomMultiPlayerGame`.
 *   - Opção 2: Permite ao utilizador selecionar um jogo específico através da função `showGames`.
 *   - Opção 3: Retorna ao menu de opções multiplayer chamando `showMultiPlayerMenu`.
 *   - Opção 4: Fecha a conexão com o servidor e termina o programa.
 * - Valida a entrada do utilizador, certificando-se de que é um número. Se não for, 
 * limpa o buffer de entrada e solicita novamente.
 * - Repete o loop até que o utilizador escolha uma opção válida (1 a 4).
 */

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

void playRandomMultiPlayerGame(int *socketfd, clientConfig config) {

    // ask server for a random game
    if (send(*socketfd, "newRandomMultiPlayerGame", strlen("newRandomMultiPlayerGame"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "can't send random multiplayer game request to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Requesting a new random multiplayer game...\n");

    }
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
                }
            }
        }
    }
}


/**
 * Envia linhas inseridas pelo utilizador ou geradas automaticamente ao servidor e 
 * processa o tabuleiro atualizado.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente, 
 * incluindo o caminho do log e o nível de dificuldade.
 *
 * @details A função realiza o seguinte:
 * - Itera por cada linha do tabuleiro, permitindo ao utilizador inserir valores manualmente ou 
 * gerando-os automaticamente com `resolveLine`.
 * - Envia cada linha ao servidor e recebe o tabuleiro atualizado.
 * - Compara a linha enviada com a linha correspondente no tabuleiro recebido para 
 * verificar se está correta.
 * - Se a linha não for correta, o utilizador é solicitado a inserir novamente ou uma 
 * nova linha é gerada automaticamente.
 * - Exibe o tabuleiro atualizado após cada tentativa, formatado de forma organizada.
 * - Regista erros de envio ou receção de dados no ficheiro de log, utilizando `err_dump`.
 */
void sendLines(int *socketfd, clientConfig config) {

    // buffer for the board
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    printf("A jogar...\n");

    char *board;
    board = showBoard(socketfd, config);

    // get the current line
    char *boardSplit = strtok(board, "\n");
    char *token = strtok(NULL, "\n");
    int currentLine = atoi(token);

    printf("Linha atual: %d\n", currentLine);

    
    EstatisticasLinha estatisticas;

    // Enviar linhas inseridas pelo utilizador e receber o board atualizado
    while (currentLine <= 9) {

        int validLine = 0;  // Variável para controlar se a linha está correta

        // line to send to server
        char line[10];

        // inicializa o buffer com '0' e terminador nulo
        memset(line, '0', sizeof(line));

        while (!validLine) {

            if (config.isManual) {

                printf("Insira valores para a linha %d do board (exactamente 9 digitos):\n", currentLine);
                scanf("%s", line);

            } else {

                // Passa a variável estatisticas para a função resolveLine
                resolveLine(boardSplit, line, currentLine - 1, config.difficulty, &estatisticas);

            }

            // Enviar a linha ao servidor
            if (send(*socketfd, line, sizeof(line), 0) < 0) {
                err_dump(config.logPath, 0, config.clientID, "can't send lines to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                continue;
            }

            printf("Linha enviada: %s\n", line);

            board = showBoard(socketfd, config);
            // get the current line
            boardSplit = strtok(board, "\n");
            char *token = strtok(NULL, "\n");
            int serverLine = atoi(token);

            printf("Linha do servidor: %d\n", serverLine);

            if (serverLine > currentLine) {
                validLine = 1;
                currentLine = serverLine;
            } else {
                printf("Linha %d incorreta. Tente novamente.\n", currentLine);
            }
        }
    }

    free(board);
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
                }
            }  
        }
    }
}


/**
 * Envia uma mensagem ao servidor para fechar a conexão e regista o evento no log.
 *
 * @param socketfd Um pointer para o descritor de socket usado para a comunicação com o servidor.
 * @param config A estrutura `clientConfig` que contém as configurações do cliente, 
 * incluindo o caminho do log e o ID do cliente.
 *
 * @details A função realiza o seguinte:
 * - Envia uma mensagem ao servidor indicando que a conexão deve ser fechada.
 * - Se o envio falhar, regista o erro no ficheiro de log utilizando `err_dump`.
 * - Se o envio for bem-sucedido, imprime uma mensagem a indicar que a conexão está a ser encerrada e regista o evento de encerramento no log.
 */

void closeConnection(int *socketfd, clientConfig config) {

    // send close connection message to the server
    if (send(*socketfd, "closeConnection", strlen("closeConnection"), 0) < 0) {
        err_dump(config.logPath, 0, config.clientID, "can't send close connection message to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {
        printf("Closing connection...\n");
        writeLogJSON(config.logPath, 0, config.clientID, EVENT_CONNECTION_CLIENT_CLOSED);
    }

    // close the socket
    close(*socketfd);

    // exit the program
    exit(0);
}

/**
 * Exibe o tabuleiro de jogo a partir de uma string JSON e regista o evento no log.
 *
 * @param buffer Uma string JSON que contém o estado do tabuleiro.
 * @param logFileName O caminho para o ficheiro de log onde o evento será registado.
 * @param playerID O identificador do jogador que está a visualizar o tabuleiro.
 *
 * @details A função faz o seguinte:
 * - Faz o parse da string JSON para obter o objeto `board` e o `gameID`.
 * - Imprime o tabuleiro no formato de uma grelha 9x9 com separadores visuais.
 * - Regista o evento de visualização do tabuleiro no ficheiro de log.
 * - Liberta a memória alocada para o objeto JSON após a operação.
 */

char *showBoard(int *socketfd, clientConfig config) {

    // buffer for the board
    // Allocate memory for buffer on the heap
    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }
    memset(buffer, 0, BUFFER_SIZE);

    printf("Received board from server...\n");

    // receive the board from the server
    if (recv(*socketfd, buffer, BUFFER_SIZE, 0) < 0) {
        // error receiving board from server
        err_dump(config.logPath, 0, config.clientID, "can't receive board from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
        free(buffer);

    } else {

        if (strcmp(buffer, "No rooms available") == 0) {
            printf("No rooms available\n");
            free(buffer);
            return NULL;
        }
    }

    printf("Board received: %s\n", buffer);

    char *board = strtok(buffer, "\n");
    char *token = strtok(NULL, "\n");
    int serverLine = atoi(token);
    printf("Linha do servidor: %d\n", serverLine);

    // get the JSON object from the buffer
    JSON_Value *root_value = json_parse_string(board);
    JSON_Object *root_object = json_value_get_object(root_value);

    // get the game ID
    int gameID = (int)json_object_get_number(root_object, "id");

    // print the board
    printf("-------------------------------------\n");
    printf("BOARD ID: %d       PLAYER ID: %d\n", gameID, config.clientID);
    printf("-------------------------------------\n");

    // get the board array from the JSON object
    JSON_Array *board_array = json_object_get_array(root_object, "board");

    for (int i = 0; i < json_array_get_count(board_array); i++) {

        // get the line array from the board array
        JSON_Array *linha_array = json_array_get_array(board_array, i);

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

    writeLogJSON(config.logPath, gameID, config.clientID, EVENT_BOARD_SHOW);

    // concatenate again board and server line
    strcat(board, "\n");
    char serverLineStr[10];
    sprintf(serverLineStr, "%d", serverLine);
    strcat(board, serverLineStr);
    strcpy(buffer, board);

    return buffer;
}