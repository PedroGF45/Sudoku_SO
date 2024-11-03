#include <pthread.h>
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs.h"
#include "server-comms.h"


static int nextPlayerID = 1;


/**
 * Gera um ID de cliente único.
 *
 * @return Um ID de cliente único, incrementando o valor de `nextPlayerID`.
 *
 * @details Esta função usa uma variável global `nextPlayerID` para gerar IDs únicos para clientes.
 * Cada vez que a função é chamada, o valor de `nextPlayerID` é retornado e, em seguida, incrementado.
 * Isto garante que cada cliente receba um ID único e sequencial.
 */

int generateUniqueClientId() {
    return nextPlayerID++;
}


/**
 * Função que gere a comunicação com um cliente ligado ao servidor.
 *
 * @param arg Um pointer genérico (void *) que é convertido para um pointer `ClientData`,
 * contendo as informações do cliente e a configuração do servidor.
 * @return Retorna NULL (a função é utilizada como um manipulador de threads, 
 * por isso o valor de retorno não é usado).
 *
 * @details Esta função faz o seguinte:
 * - Obtém os dados do cliente a partir do argumento `arg` e inicializa as variáveis necessárias.
 * - Recebe o pedido de ID do cliente, gera um ID único, e envia-o de volta ao cliente.
 * - Entra num ciclo principal onde recebe comandos do cliente e executa as ações correspondentes:
 *   - Criar um novo jogo single player ou multiplayer.
 *   - Listar e selecionar jogos ou salas existentes.
 *   - Receber e enviar dados relacionados com o estado do jogo e ações do cliente.
 * - Gere a comunicação com o cliente, incluindo o envio e a receção de mensagens, 
 *   e regista eventos no ficheiro de log.
 * - Trata erros de forma apropriada, terminando a conexão e a thread se necessário.
 * - Fecha a ligação com o cliente, liberta a memória alocada e termina a execução da thread.
 */

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


/**
 * Cria uma nova sala de jogo e carrega um jogo associado, atribuindo-o a um jogador.
 *
 * @param newSockfd Um pointer para o descritor de socket do cliente, usado para enviar mensagens.
 * @param config Um pointer para a estrutura `ServerConfig` que contém as configurações do servidor.
 * @param playerID O identificador do jogador que irá participar na sala.
 * @param isSinglePlayer Um valor booleano que indica se o jogo é single player (true) ou multiplayer (false).
 * @param isRandom Um valor booleano que indica se o jogo deve ser carregado aleatoriamente (true),
 * ou se um jogo específico deve ser carregado (false).
 * @param gameID O identificador do jogo a ser carregado, usado se `isRandom` for false.
 * @return Um pointer para a estrutura `Room` criada, ou NULL se não houver salas disponíveis.
 *
 * @details Esta função faz o seguinte:
 * - Verifica se o número máximo de salas foi atingido. 
 *   Se não houver salas disponíveis, envia uma mensagem ao cliente e regista o evento no ficheiro de log.
 * - Cria uma nova sala e adiciona-a à lista de salas do servidor.
 * - Carrega um jogo, seja de forma aleatória ou usando o `gameID` especificado.
 * - Se o jogo for carregado com sucesso, associa-o à sala criada.
 * - Define o número máximo de jogadores para 1 se for um jogo single player, 
 *   ou para o valor definido na configuração se for multiplayer.
 * - Adiciona o jogador à sala e inicializa o número de jogadores da sala.
 * - Imprime uma mensagem de confirmação e retorna a sala criada.
 */

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


/**
 * Inicializa o socket do servidor, associando-o a um endereço e porta especificados na configuração.
 *
 * @param serv_addr Um pointer para a estrutura `sockaddr_in` que será preenchida com as informações do endereço do servidor.
 * @param sockfd Um pointer para o descritor de socket, que será inicializado e associado ao endereço.
 * @param config Um pointer para a estrutura `ServerConfig` que contém as configurações do servidor, incluindo a porta.
 *
 * @details Esta função faz o seguinte:
 * - Cria um socket do tipo `SOCK_STREAM` para comunicações baseadas em TCP.
 * - Limpa a estrutura do socket com `memset` para garantir que todos os campos são inicializados corretamente.
 * - Preenche a estrutura do socket com a família de endereços `AF_INET`, 
 *   o endereço `INADDR_ANY` (para aceitar conexões de qualquer endereço) e a porta especificada na configuração do servidor.
 * - Usa `bind` para associar o socket ao endereço e porta definidos. Se a operação falhar, 
 *   a função regista um erro no log e termina o programa.
 * - Coloca o socket em modo de escuta com a função `listen`, permitindo que o servidor aceite conexões.
 */

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


/**
 * Envia o tabuleiro do jogo ao cliente em formato JSON.
 *
 * @param socket Um pointer para o descritor de socket utilizado para enviar o tabuleiro ao cliente.
 * @param game Um pointer para a estrutura `Game` que contém o tabuleiro a ser enviado.
 * @param config Um pointer para a estrutura `ServerConfig` que contém a configuração do servidor, 
 * incluindo o caminho do ficheiro de log.
 *
 * @details Esta função faz o seguinte:
 * - Cria uma estrutura JSON que representa o tabuleiro do jogo, incluindo o ID do jogo.
 * - Converte o tabuleiro 9x9 num array de arrays em formato JSON.
 * - Serializa o objeto JSON para uma string e envia-a ao cliente através do socket.
 * - Em caso de erro ao enviar o tabuleiro, regista a mensagem de erro no log e termina a execução da função.
 * - Liberta a memória alocada para a string serializada e o objeto JSON.
 */

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


/**
 * Recebe linhas do cliente, valida-as, e atualiza o tabuleiro do jogo.
 *
 * @param newSockfd Um pointer para o descritor de socket usado para comunicação com o cliente.
 * @param game Um pointer para a estrutura `Game` que contém o estado atual do tabuleiro.
 * @param playerID O identificador do jogador que está a enviar as linhas.
 * @param config Um pointer para a estrutura `ServerConfig` que contém a configuração do servidor, 
 * incluindo o caminho do ficheiro de log.
 *
 * @details Esta função faz o seguinte:
 * - Itera sobre as 9 linhas do tabuleiro, recebendo e validando cada linha enviada pelo cliente.
 * - Recebe cada linha do cliente como uma string de 9 caracteres e converte-a em valores inteiros.
 * - Usa a função `verifyLine` para validar a linha recebida. Se a linha estiver correta, 
 *   continua para a próxima; caso contrário, solicita ao cliente que envie novamente.
 * - Se ocorrer um erro ao receber uma linha, regista o erro no ficheiro de log e termina a execução da função.
 * - Após cada validação, envia o tabuleiro atualizado ao cliente.
 * - Adiciona um atraso de 1 segundo (`sleep(1)`) antes de enviar o tabuleiro para garantir que o cliente tem 
 *   tempo para processar as atualizações.
 */

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


/**
 * Termina o jogo e limpa os recursos associados à sala de jogo.
 *
 * @param socket Um pointer para o descritor de socket utilizado para comunicação com o cliente 
 * (não usado diretamente na função).
 * @param room Um pointer para a estrutura `Room` que representa a sala de jogo que deve ser terminada.
 * @param playerID O identificador do jogador (não usado diretamente na função).
 * @param config Um pointer para a estrutura `ServerConfig` que contém a configuração do servidor, 
 * incluindo o número atual de salas.
 *
 * @details Esta função faz o seguinte:
 * - Remove todos os jogadores da sala, definindo os IDs dos jogadores para 0.
 * - Restaura o número de jogadores da sala e o número máximo de jogadores a 0.
 * - Liberta a memória alocada para o jogo e a sala, prevenindo fugas de memória.
 * - Decrementa o contador do número de salas no servidor na configuração.
 */

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
