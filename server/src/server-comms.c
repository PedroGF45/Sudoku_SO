#include <pthread.h>
#include <sys/select.h>
#include <time.h>
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs-common.h"
#include "server-comms.h"
#include "../logs/logs.h"


static int nextClientID = 1;

void sendRoomStatistics(ServerConfig *config, Client *client) {
    FILE *file = fopen("room_stats.log", "r");
    if (file == NULL) {
        const char *errorMsg = "Erro: Não foi possível abrir o ficheiro de estatísticas.\n";
        if (send(client->socket_fd, errorMsg, strlen(errorMsg), 0) < 0) {
            // erro ao enviar mensagem de erro
            err_dump(config, 0, client->clientID, "can't send error message to client", EVENT_MESSAGE_SERVER_NOT_SENT);
        } else {
            printf("Erro: Não foi possível abrir o ficheiro de estatísticas\n");
            produceLog(config, "Erro: Não foi possível abrir o ficheiro de estatísticas", EVENT_MESSAGE_SERVER_SENT, 0, client->clientID);
        }
        return;
    }

    char line[256];
    char stats[1024] = "";  // Buffer para enviar todas as estatísticas
    while (fgets(line, sizeof(line), file) != NULL) {
        strncat(stats, line, sizeof(stats) - strlen(stats) - 1);  // Adiciona cada linha ao buffer
    }
    fclose(file);

    // if there are no statistics send "No statistics available"
    if (strlen(stats) == 0) {
        strcpy(stats, "No statistics available");
    }
        

    // Envia as estatísticas ao cliente
    if (send(client->socket_fd, stats, strlen(stats), 0) < 0) {
        // erro ao enviar estatísticas
        err_dump(config, 0, client->clientID, "can't send statistics to client", EVENT_MESSAGE_SERVER_NOT_SENT);
    } else {
        printf("Estatísticas enviadas ao cliente\n");
        produceLog(config, "Estatísticas enviadas ao cliente", EVENT_MESSAGE_SERVER_SENT, 0, client->clientID);
    }
}
/**
 * Gera um ID de cliente único.
 *
 * @return Um ID de cliente único, incrementando o valor de `nextClientID`.
 *
 * @details Esta função usa uma variável global `nextClientID` para gerar IDs únicos para clientes.
 * Cada vez que a função é chamada, o valor de `nextClientID` é retornado e, em seguida, incrementado.
 * Isto garante que cada cliente receba um ID único e sequencial.
 */

int generateUniqueClientId() {
    return nextClientID++;
}

/**
 * Função que gere a comunicação com um cliente ligado ao servidor.
 *
 * @param arg Um pointer genérico (void *) que é convertido para um pointer `client`,
 * contendo as informações do cliente e a configuração do servidor.
 * @return Retorna NULL (a função é utilizada como um manipulador de threads, 
 * por isso o valor de retorno não é usado).
 *
 * @details Esta função faz o seguinte:
 * - Obtém os dados do cliente a partir do argumento `arg` e inicializa as variáveis necessárias.
 * - Recebe o pedido de ID do cliente, gera um ID único, e envia-o de volta ao cliente.
 * - Entra num ciclo principal onde recebe comandos do cliente e executa as ações correspondentes:
 *   - Criar um novo jogo single Client ou MultiPlayer.
 *   - Listar e selecionar jogos ou salas existentes.
 *   - Receber e enviar dados relacionados com o estado do jogo e ações do cliente.
 * - Gere a comunicação com o cliente, incluindo o envio e a receção de mensagens, 
 *   e regista eventos no ficheiro de log.
 * - Trata erros de forma apropriada, terminando a conexão e a thread se necessário.
 * - Fecha a ligação com o cliente, liberta a memória alocada e termina a execução da thread.
 */

void *handleClient(void *arg) {

    // Obter dados do cliente
    client_data *data = (client_data *)arg;
    Client *client = data->client;
    ServerConfig *serverConfig = data->config;
    int newSockfd = client->socket_fd;

    // receber buffer do cliente
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    Room *room;
    int currentLine = 1;

    //printf("A AGUARDAR POR PEDIDOS DO CLIENTE\n");

    // receber premium status
    if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
        // erro ao receber status premium
        err_dump(serverConfig, 0, client->clientID, "can't receive premium status", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
    } else if (strcmp(buffer, "premium") == 0) {
        client->isPremium = true;
    } else {
        client->isPremium = false;
    }

    // send id to client
    client->clientID = generateUniqueClientId();
    sprintf(buffer, "%d", client->clientID);

    if (send(newSockfd, buffer, strlen(buffer), 0) < 0) {
        // erro ao enviar ID do jogador
        err_dump(serverConfig, 0, 0, "can't send client ID", EVENT_MESSAGE_SERVER_NOT_SENT);
    } else {
        printf("ID atribuido ao novo cliente: %d (%s)\n", client->clientID, client->isPremium ? "premium" : "not premium");
        produceLog(serverConfig, "Conexao estabelecida com um novo cliente", EVENT_CONNECTION_SERVER_ESTABLISHED, 0, client->clientID);
    }
    
    bool continueLoop = true;

    while (continueLoop) {

        printf("A aguardar por pedidos do cliente %d\n", client->clientID);

        client->startAgain = false;
        memset(buffer, 0, sizeof(buffer));

        // Receber menu status
        if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
            // erro ao receber modo de jogo
            err_dump(serverConfig, 0, client->clientID, "can't receive menu status", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
        } else {

            printf("BUFFER RECEBIDO: %s\n", buffer);
            
            // cliente quer ver as estatisticas
            if(strcmp(buffer, "GET_STATS") == 0){

                pthread_mutex_lock(&serverConfig->mutex);

                sendRoomStatistics(serverConfig, client);
                client->startAgain = true;

                pthread_mutex_unlock(&serverConfig->mutex);

            } else if (strcmp(buffer, "newSinglePlayerGame") == 0) {

                pthread_mutex_lock(&serverConfig->mutex);

                // criar novo jogo single player
                room = createRoomAndGame(serverConfig, client, true, true, 0, 0);

                pthread_mutex_unlock(&serverConfig->mutex);

            } else if(strcmp(buffer, "newMultiPlayerGameReadersWriters") == 0) {

                printf("Cliente %d solicitou um novo jogo rando multiplayer game com readers-writers\n", client->clientID);

                // lock mutex
                pthread_mutex_lock(&serverConfig->mutex);

                room = createRoomAndGame(serverConfig, client, false, true, 0, 0);

                // unlock mutex
                pthread_mutex_unlock(&serverConfig->mutex);

                // adicionar timer
                handleTimer(serverConfig, room, client);

            } else if (strcmp(buffer, "newMultiPlayerGameBarberShopStaticPriority") == 0) {

                
                pthread_mutex_lock(&serverConfig->mutex);

                room = createRoomAndGame(serverConfig, client, false, true, 0, 1);

                pthread_mutex_unlock(&serverConfig->mutex);

                // adicionar timer
                handleTimer(serverConfig, room, client);

            } else if (strcmp(buffer, "newMultiPlayerGameBarberShopDynamicPriority") == 0) {

                printf("Cliente %d solicitou um novo jogo rando multiplayer game com barber shop dynamic priority\n", client->clientID);

                // lock mutex
                pthread_mutex_lock(&serverConfig->mutex);

                room = createRoomAndGame(serverConfig, client, false, true, 0, 2);

                // unlock mutex
                pthread_mutex_unlock(&serverConfig->mutex);

                // adicionar timer
                handleTimer(serverConfig, room, client);

            } else if (strcmp(buffer, "newMultiPlayerGameBarberShopFIFO") == 0) {

                printf("Cliente %d solicitou um novo jogo rando multiplayer game com barber shop fifo\n", client->clientID);

                // lock mutex
                pthread_mutex_lock(&serverConfig->mutex);

                room = createRoomAndGame(serverConfig, client, false, true, 0, 3);

                // unlock mutex
                pthread_mutex_unlock(&serverConfig->mutex);

                // adicionar timer
                handleTimer(serverConfig, room, client);

            } else if (strcmp(buffer, "selectSinglePlayerGames") == 0 || strcmp(buffer, "selectMultiPlayerGames") == 0) {

                bool isSinglePlayer = strncmp(buffer, "selectSinglePlayerGames", strlen("selectSinglePlayerGames")) == 0;
                // Receber jogos existentes
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));

                // lock mutex
                pthread_mutex_lock(&serverConfig->mutex);
                // Obter jogos existentes
                char *games = getGames(serverConfig);

                printf("Jogos existentes: %s\n", games);

                // unlock mutex
                pthread_mutex_unlock(&serverConfig->mutex);

                bool leave = false;

                while (!leave) {
                    // Enviar jogos existentes ao cliente
                    if (send(newSockfd, games, strlen(games), 0) < 0) {
                        free(games);
                        err_dump(serverConfig, 0, client->clientID, "can't send existing games to client", EVENT_MESSAGE_SERVER_NOT_SENT);
                    } else {
                        produceLog(serverConfig, "Jogos enviados para o cliente", EVENT_SERVER_GAMES_SENT, 0, client->clientID);
                    }

                    memset(buffer, 0, sizeof(buffer));

                    // receber ID do jogo
                    if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
                        free(games);
                        err_dump(serverConfig, 0, client->clientID, "can't receive game ID from client", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
                    } else if (atoi(buffer) == 0) {
                        printf("Cliente %d voltou atras no menu\n", client->clientID);
                        leave = true;
                        client->startAgain = true;
                    } else {
                          printf("Cliente %d escolheu o jogo com o ID: %s\n", client->clientID, buffer);

                        // obter ID do jogo
                        int gameID = atoi(buffer);

                        int synchronizationType;

                        // need to receive synchronization type
                        if (!isSinglePlayer) {

                            // receive synchronization type
                            memset(buffer, 0, sizeof(buffer));

                            if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
                                    err_dump(serverConfig, 0, client->clientID, "can't receive synchronization type from client", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
                            } else {
                                printf("BUFFER RECEBIDO PARA SYNCHRONIZATION TYPE: %s\n", buffer);
                                if (strcmp(buffer, "newMultiPlayerGameReadersWriters") == 0) {
                                    printf("Cliente %d escolheu o jogo com readers-writers\n", client->clientID);
                                    synchronizationType = 0;
                                } else if (strcmp(buffer, "newMultiPlayerGameBarberShopStaticPriority") == 0) {
                                    printf("Cliente %d escolheu o jogo com barber shop priority\n", client->clientID);
                                    synchronizationType = 1;
                                } else if (strcmp(buffer, "newMultiPlayerGameBarberShopDynamicPriority") == 0) {
                                    printf("Cliente %d escolheu o jogo com barber shop dynamic priority\n", client->clientID);
                                    synchronizationType = 2;
                                } else if (strcmp(buffer, "newMultiPlayerGameBarberShopFIFO") == 0) {
                                    printf("Cliente %d escolheu o jogo com barber shop fifo\n", client->clientID);
                                    synchronizationType = 3;
                                } else if (strcmp(buffer, "0") == 0) {
                                    printf("Cliente %d voltou atras no menu\n", client->clientID);
                                    leave = true;
                                    client->startAgain = true;
                                    break;
                                }
                            }

                            //printf("Synchronization type: %d\n", synchronizationType);
                        }

                        // lock mutex
                        pthread_mutex_lock(&serverConfig->mutex);

                        room = createRoomAndGame(serverConfig, client, isSinglePlayer, false, gameID, synchronizationType);

                        // unlock mutex
                        pthread_mutex_unlock(&serverConfig->mutex);

                        if (!isSinglePlayer) {
                            // adicionar timer
                            handleTimer(serverConfig, room, client);
                        }

                        leave = true;
                    }
                }

                free(games);
                games = NULL;

            } else if (strcmp(buffer, "existingRooms") == 0) {
                  // Receber jogos existentes
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));
                // Obter salas existentes

                pthread_mutex_lock(&serverConfig->mutex);

                char *rooms = getRooms(serverConfig);

                printf("Rooms existentes: %s\n", rooms);

                pthread_mutex_unlock(&serverConfig->mutex);

                bool leave = false;

                while (!leave) {
                    // Enviar salas existentes ao cliente
                    if (send(newSockfd, rooms, strlen(rooms), 0) < 0) {
                        free(rooms);
                        err_dump(serverConfig, 0, client->clientID, "can't send existing games to client", EVENT_MESSAGE_SERVER_NOT_SENT);
                    } else {
                        produceLog(serverConfig, "Jogos enviados para o cliente", EVENT_SERVER_GAMES_SENT, 0, client->clientID);
                    }
                    printf("RECEBER IDS DAS ROOMS\n");
                    memset(buffer, 0, sizeof(buffer));

                    // receber ID da sala
                    if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
                        free(rooms);
                        err_dump(serverConfig, 0, client->clientID, "can't receive room ID from client", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
                    } else if (atoi(buffer) == 0) {
                        printf("Cliente %d voltou atras no menu\n", client->clientID);
                        leave = true;
                        client->startAgain = true;
                    } else {
                        int roomID = atoi(buffer);

                        //printf("ROOM ID NO SERVIDOR: %d\n", roomID);

                        // lock mutex
                        pthread_mutex_lock(&serverConfig->mutex);

                        // get the room
                        room = getRoom(serverConfig, roomID, client->clientID);

                        // unlock mutex
                        pthread_mutex_unlock(&serverConfig->mutex);

                        // add the Client to the queue
                        printf("ENQUEING CLIENT %d which is %s\n", client->clientID, client->isPremium ? "premium" : "not premium");
                        enqueueWithPriority(room->enterRoomQueue, client->clientID, client->isPremium);

                        //printf("BEFORE SLEEP\n");

                        // sleep thread for test
                        sleep(5);

                        //printf("AFTER SLEEP\n");

                        // Join room all clients in queue
                        pthread_mutex_lock(&room->mutex);


                        for (int i = 0; i < room->maxClients * 2; i++) {

                            if (room->enterRoomQueue->front == NULL) {
                                break;

                            } else {

                                Client *clientTemp;

                                int clientIDJoin = dequeue(room->enterRoomQueue);

                                // get client socket
                                for (int i = 0; i < serverConfig->numClientsOnline; i++) {
                                    if (serverConfig->clients[i] != NULL && serverConfig->clients[i]->clientID == clientIDJoin) {
                                        clientTemp = serverConfig->clients[i];
                                        break;
                                    }
                                }

                                // check if room is full so the client doesnt join
                                printf("CHECKING IF ROOM IS FULL FOR CLIENT %d\n", clientTemp->clientID);
                                if (room->numClients >= room->maxClients) {
                                    
                                    // send message to client
                                    if (send(clientTemp->socket_fd, "Room is full", strlen("Room is full"), 0) < 0) {
                                        err_dump(serverConfig, 0, client->clientID, "can't send message to client", EVENT_MESSAGE_SERVER_NOT_SENT);
                                    } else {
                                        produceLog(serverConfig, "Room is full", EVENT_ROOM_NOT_JOIN, 0, clientTemp->clientID);
                                    }

                                    clientTemp->startAgain = true;

                                } else {
                                    
                                    joinRoom(serverConfig, room, clientTemp);
                                }
                            }
                        }

                        pthread_mutex_unlock(&room->mutex);

                        // atualizar tempo de espera para este Client
                        handleTimer(serverConfig, room, client);

                        leave = true;
 
                    }
                }
                //printf("Rooms pointer before free: %p\n", (void*)rooms);
                free(rooms);
                //rooms = NULL;

            } else if (strcmp(buffer, "closeConnection") == 0) {
                continueLoop = false;
                break;
            } else if (strcmp(buffer, "0") == 0) {
                printf("Cliente %d voltou atras no menu\n", client->clientID);
                client->startAgain = true;
            }

            if (!client->startAgain) {
                room->isGameRunning = true;
                room->startTime = time(NULL);

                // barreira para começar o jogo
                if (!room->isSinglePlayer) {//Se o jogo for multiplayer
                    acquireTurnsTileSemaphore(room, client);//Garante que todas as threads estao prontas antes de avançar para a fase critica
                }

                // receber linhas do cliente
                receiveLines(serverConfig, room, client, &currentLine);

                if (!room->isSinglePlayer) {
                    // barreira para terminar o jogo
                    releaseTurnsTileSemaphore(room, client);
                }

                // mutex para terminar o jogo
                pthread_mutex_lock(&serverConfig->mutex);
                // terminar o jogo
                finishGame(serverConfig, room, &newSockfd);

                // unlock mutex
                pthread_mutex_unlock(&serverConfig->mutex);
            }
        }
    }

    // fechar a ligação com o cliente
    close(newSockfd);
    printf("Conexao terminada com o cliente %d\n", client->clientID);
    produceLog(serverConfig, "Conexao terminada com o cliente", EVENT_SERVER_CONNECTION_FINISH, 0, client->clientID);

    // libertar a memória alocada
    removeClient(serverConfig, client);
    free(data);

    // terminar a thread
    pthread_exit(NULL);
}


/**
 * Cria uma nova sala de jogo e carrega um jogo associado, atribuindo-o a um jogador.
 *
 * @param newSockfd Um pointer para o descritor de socket do cliente, usado para enviar mensagens.
 * @param config Um pointer para a estrutura `ServerConfig` que contém as configurações do servidor.
 * @param ClientID O identificador do jogador que irá participar na sala.
 * @param isSinglePlayer Um valor booleano que indica se o jogo é single Client (true) ou MultiPlayer (false).
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
 * - Define o número máximo de jogadores para 1 se for um jogo single Client, 
 *   ou para o valor definido na configuração se for MultiPlayer.
 * - Adiciona o jogador à sala e inicializa o número de jogadores da sala.
 * - Imprime uma mensagem de confirmação e retorna a sala criada.
 */

Room *createRoomAndGame(ServerConfig *config, Client *client, bool isSinglePlayer, bool isRandom, int gameID, int synchronizationType) {

    // check if there's config->rooms is full by looping
    if (config->numRooms >= config->maxRooms) {
        // send message to client
        send(client->socket_fd, "No rooms available", strlen("No rooms available"), 0);
        // write log
        produceLog(config, "No rooms available", EVENT_ROOM_NOT_CREATED, 0, client->clientID);
        return NULL;
    }

    // create room
    Room *room = createRoom(config, client->clientID, isSinglePlayer, synchronizationType);

    // load game
    Game *game;

    if (isRandom) {
        game = loadRandomGame(config, client->clientID);
    } else {
        game = loadGame(config, gameID, client->clientID);
    }

    if (game == NULL) {
        fprintf(stderr, "Error loading random game\n");
        exit(1);
    }

    // add game to room
    room->game = game;
    
    joinRoom(config, room, client);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    strcpy(buffer, "");

    if (!room->isSinglePlayer && !room->isReaderWriter) {
        if (room->priorityQueueType == 0) {
            strcpy(buffer, " and STATIC PRIORITIES queue");
        } else if (room->priorityQueueType == 1) {
            strcpy(buffer, " and DYNAMIC PRIORITIES queue");
        } else if (room->priorityQueueType == 2) {
            strcpy(buffer, " and FIFO queue");
        }
    }

    // Mensagem de criação da sala
    printf("New game created by client %d with game %d and room is synchronized by %s%s. Client %d is %s.\n", 
            client->clientID, room->game->id, 
            !room->isSinglePlayer ? (room->isReaderWriter ? "READER-WRITER" : "BARBER SHOP") : "",
            buffer,
            client->clientID, client->isPremium ? "Premium" : "Non-premium");
    
    if (!room->isSinglePlayer) {
        printf("Waiting for more Clients to join\n");
    }
    

    return room;
}

/**
 * Adiciona um jogador a uma sala de jogo existente.
 *
 * @param config Um pointer para a estrutura `ServerConfig` que contém as informações 
 * de configuração do servidor, incluindo a lista de salas.
 * @param roomID O identificador da sala que o jogador pretende entrar.
 * @param ClientID O identificador do jogador que deseja juntar-se à sala.
 * @return Um pointer para a sala `Room` se o jogador for adicionado com sucesso, 
 * ou NULL se a sala estiver cheia ou se o `roomID` for inválido.
 *
 * @details Esta função faz o seguinte:
 * - Verifica se o `roomID` fornecido é válido (dentro dos limites das salas existentes).
 * - Obtém a sala correspondente a partir da lista de salas no servidor.
 * - Verifica se a sala já está cheia. Se estiver, devolve NULL e imprime uma mensagem de erro no terminal.
 * - Se houver espaço na sala, adiciona o jogador ao array de jogadores da sala e incrementa o número de jogadores.
 * - Regista no ficheiro de log que o jogador entrou na sala.
 * - Devolve um pointer para a sala `Room` atualizada.
 */
void joinRoom(ServerConfig *config, Room *room, Client *client) {

    // check if room is full
    if (room->numClients == room->maxClients) {
        err_dump(config, 0, client->clientID, "Room is full", EVENT_ROOM_NOT_JOIN);
        return;
    }

    // check if room is running
    if (room->isGameRunning) {
        err_dump(config, 0, client->clientID, "Room is running", EVENT_ROOM_NOT_JOIN);
        return;
    }

    printf("Client %d joined room %d\n", client->clientID, room->id);

    // add client to room
    room->clients[room->numClients] = client;

    // increment number of clients
    room->numClients++;

    // Log de entrada do jogador na sala
    char logMessage[256];
    snprintf(logMessage, sizeof(logMessage), "Client %d joined room %d", client->clientID, room->id);
    produceLog(config, logMessage, EVENT_ROOM_JOIN, room->game->id, client->clientID);

    printf("Client %d (Premium: %s) joined room %d with socket %d\n",
           client->clientID, client->isPremium ? "Yes" : "No", room->id, client->socket_fd);
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
        err_dump(config, 0, 0, "can't open stream socket", EVENT_CONNECTION_SERVER_ERROR);
    }

    // Limpar a estrutura do socket
    memset((char *) serv_addr, 0, sizeof(*serv_addr));

    // Preencher a estrutura do socket
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr->sin_port = htons(config->serverPort);

    // Associar o socket a um endereco qualquer
    if (bind(*sockfd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        err_dump(config, 0, 0, "can't bind local address", EVENT_CONNECTION_SERVER_ERROR);
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

void sendBoard(ServerConfig *config, Room* room, Client *client) {

    // Enviar board ao cliente em formato JSON
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, "id", room->game->id);
    JSON_Value *board_value = json_value_init_array();
    JSON_Array *board_array = json_value_get_array(board_value);

    for (int i = 0; i < 9; i++) {
        JSON_Value *linha_value = json_value_init_array();
        JSON_Array *linha_array = json_value_get_array(linha_value);
        for (int j = 0; j < 9; j++) {
            json_array_append_number(linha_array, room->game->board[i][j]);
        }
        json_array_append_value(board_array, linha_value);
    }

    json_object_set_value(root_object, "board", board_value);
    char *serialized_string = json_serialize_to_string(root_value);

    //adicionar a linha atual como um inteiro à string
    char buffer[10];
    sprintf(buffer, "\n%d", room->game->currentLine);
    char *temp = malloc(strlen(serialized_string) + strlen(buffer) + 1);
    strcpy(temp, serialized_string);
    strcat(temp, buffer);

    //printf("Enviando tabuleiro ao cliente %d do jogo %d\n", client->clientID, room->game->id);
    //printf("Enviando board e linha atual: %s\n", temp);
    // Enviar tabuleiro e linha atual ao cliente
    if (send(client->socket_fd, temp, strlen(temp), 0) < 0) {
        err_dump(config, room->game->id, 0, "can't send board and line to client", EVENT_MESSAGE_SERVER_NOT_SENT);
        return;
    }

    // escrever no log
    produceLog(config, "Tabuleiro enviado ao cliente", EVENT_MESSAGE_SERVER_SENT, room->game->id, client->clientID);

    free(temp);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}


/**
 * Recebe linhas do cliente, valida-as, e atualiza o tabuleiro do jogo.
 *
 * @param newSockfd Um pointer para o descritor de socket usado para comunicação com o cliente.
 * @param game Um pointer para a estrutura `Game` que contém o estado atual do tabuleiro.
 * @param ClientID O identificador do jogador que está a enviar as linhas.
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

void receiveLines(ServerConfig *config, Room *room, Client *client, int *currentLine) {

    // pre condition reader
    if (!room->isSinglePlayer) {
        if (room->isReaderWriter) {
            acquireReadLock(room);
        } else {
            enterBarberShop(room, client);
        }
    }

    int correctLine = 0;

    // critical section reader
    sendBoard(config, room, client);

    // post condition reader
    if (!room->isSinglePlayer) {
        if (room->isReaderWriter) {
            releaseReadLock(room);
        } else {
            leaveBarberShop(room, client);
        }
    }

    // Receber e validar as linhas do cliente
    while (room->game->currentLine <= 9) {

        //printf("Recebendo linha %d do cliente %d\n", *currentLine, ClientID);

        char line[10];

        // Limpar linha
        memset(line, '0', sizeof(line));

        // Receber linha do cliente
        if (recv(client->socket_fd, line, sizeof(line), 0) < 0) {
            err_dump(config, room->game->id, client->clientID, "can't receive line from client", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
            return;
        } else {

            printf("Cliente %d %s quer resolver a linha %d\n", client->clientID, client->isPremium ? "(PREMIUM)" : "", room->game->currentLine);

            // pre condition writer
            if (!room->isSinglePlayer) {
                if (room->isReaderWriter) {
                    acquireWriteLock(room, client);
                } else {
                    enterBarberShop(room, client);
                }
            }

            printf("-----------------------------------------------------\n");

            // **Adicionei print para verificar a linha recebida**
            //printf("Linha recebida do cliente %d: %s\n", client->clientID, line); 

            // Converte a linha recebida em valores inteiros
            int insertLine[9];
            for (int j = 0; j < 9; j++) {
                insertLine[j] = line[j] - '0';
            }

            printf("Verificando linha %d do cliente %d\n", room->game->currentLine, client->clientID);
            // critical section writer
            // Verificar a linha recebida com a função verifyLine
            correctLine = verifyLine(config, room->game, line, insertLine, client->clientID);

            if (correctLine == 1) {
                // linha correta
                //printf("Linha %d correta enviada pelo cliente %d\n", room->game->currentLine, client->clientID);
                (room->game->currentLine)++;

            } else {
                // linha incorreta
                //printf("Linha %d incorreta enviada pelo cliente %d\n", room->game->currentLine, client->clientID);
            }

            // post condition writer
            if (!room->isSinglePlayer) {
                if (room->isReaderWriter) {
                    releaseWriteLock(room, client);
                } else {
                    leaveBarberShop(room, client);
                }
            }

            // add a random delay to appear more natural
            //int delay = rand() % 3;
            //sleep(delay);
            
            // pre condition reader
            if (!room->isSinglePlayer) {
                if (room->isReaderWriter) {
                    acquireReadLock(room);
                } else {
                    enterBarberShop(room, client);
                }
            }

            sendBoard(config, room, client);
            printf("-----------------------------------------------------\n");
            // post condition reader
            if (!room->isSinglePlayer) {
                if (room->isReaderWriter) {
                    releaseReadLock(room);
                } else {
                    leaveBarberShop(room, client);
                }
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
 * @param ClientID O identificador do jogador (não usado diretamente na função).
 * @param config Um pointer para a estrutura `ServerConfig` que contém a configuração do servidor, 
 * incluindo o número atual de salas.
 *
 * @details Esta função faz o seguinte:
 * - Remove todos os jogadores da sala, definindo os IDs dos jogadores para 0.
 * - Restaura o número de jogadores da sala e o número máximo de jogadores a 0.
 * - Liberta a memória alocada para o jogo e a sala, prevenindo fugas de memória.
 * - Decrementa o contador do número de salas no servidor na configuração.
 */

void finishGame(ServerConfig *config, Room *room, int *socket) {

    // lock mutex
    pthread_mutex_lock(&room->mutex);

    if (room == NULL) {
        return;
    }

    if (!room->isFinished) {
        time_t endTime = time(NULL);
        room->elapsedTime = difftime(endTime, room->startTime);
    
        // check if is single Client
        if (!room->isSinglePlayer) {
            // Substract 60 seconds from elapsed time
            room->elapsedTime = room->elapsedTime;
        }

        printf("Jogo na sala %d terminou. Tempo total: %.2f segundos\n", room->id, room->elapsedTime);

        // for every Client in the room
        for (int i = 0; i < room->numClients; i++) {

            // get accuracy from client
            char accuracy[10];
            memset(accuracy, 0, sizeof(accuracy));
            if (recv(room->clients[i]->socket_fd, accuracy, sizeof(accuracy), 0) < 0) {
                // erro ao receber accuracy
                err_dump(config, room->game->id, room->clients[i]->clientID, "can't receive accuracy from client", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
            }

            printf("A accuracy recebida foi de: %s\n", accuracy);

            // convert accuracy to float
            float accuracyFloat = atof(accuracy);

            char timeMessage[256];
            snprintf(timeMessage, sizeof(timeMessage), "A accuracy recebida foi de: %.2f %%\n", accuracyFloat);
            produceLog(config, timeMessage, EVENT_MESSAGE_SERVER_RECEIVED, room->game->id, room->clients[i]->clientID);

            // Envia o tempo decorrido ao cliente
            
            snprintf(timeMessage, sizeof(timeMessage), "O jogo terminou! Tempo total: %.2f segundos\n", room->elapsedTime);
            if (send(room->clients[i]->socket_fd, timeMessage, strlen(timeMessage), 0) < 0) {
                // erro ao enviar mensagem
                err_dump(config, room->game->id, room->clients[i]->clientID, "can't send time message to client", EVENT_MESSAGE_SERVER_NOT_SENT);
            }

            // escrever no log timeMessage with EVENT_MESSAGE_SERVER_SENT
            snprintf(timeMessage, sizeof(timeMessage), "Time elapsed: %.2f seconds", room->elapsedTime);
            produceLog(config, timeMessage, EVENT_MESSAGE_SERVER_SENT, room->game->id, room->clients[i]->clientID);

            // save on games.json the record
            updateGameStatistics(config, room->game->id, room->elapsedTime, accuracyFloat);

            // Save room statistics in log
            saveRoomStatistics(room->id, room->elapsedTime);
        }

        // set room as finished
        room->isFinished = true;

        // remove room
        deleteRoom(config, room->id);
    }

    // unlock mutex
    pthread_mutex_unlock(&room->mutex);
  
}  

void handleTimer(ServerConfig *config, Room *room, Client *client) {
    fd_set readfds;
    struct timeval tv;

    // lock mutex
    pthread_mutex_lock(&room->timerMutex);

    if (room->timer == 0) {
        pthread_mutex_unlock(&room->timerMutex);
        return;
    }

    while (room->timer > 0) {
        FD_ZERO(&readfds);
        FD_SET(client->socket_fd, &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int result = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);
        //printf("Timer: %d\n", room->timer);
        //printf("Result: %d\n", result);
        //printf("Current thread: %ld\n", pthread_self());

        if (result > 0) {
            // Recebe mensagem do cliente (se necessário)
        } else if (result == 0) {
            if (room->timer == 0) {
                break;
            }

            // Verificar se todos os jogadores se juntaram
            if (room->numClients == room->maxClients) {
                pthread_mutex_lock(&room->mutex);
                room->timer = 0;
                printf("All Clients have joined the room %d\n", room->id);
                printf("Starting game in room %d\n", room->id);

                // Enviar atualização do timer para todos os jogadores
                for (int i = 0; i < room->numClients; i++) {
                    
                    //printf("Sending timer update to Client %d with the socket: %d\n", room->Clients[i], room->clientSockets[i]);
                    sendTimerUpdate(config, room, room->clients[i]);
                    printf("Sent timer update to Client %d on socket %d\n", room->clients[i]->clientID, room->clients[i]->socket_fd);
                    
                }
                pthread_mutex_unlock(&room->mutex);
                break;
            }

            // Enviar atualização do timer a cada 10 segundos ou quando o timer for inferior a 5 segundos
            if (room->timer % 10 == 0 || room->timer <= 5) {
                //pthread_mutex_lock(&room->mutex);
                for (int i = 0; i < room->numClients; i++) {
                    //printf("Sending timer update to Client %d with the socket: %d\n", room->Clients[i], room->clientSockets[i]);
                    sendTimerUpdate(config, room, room->clients[i]);
                }
                //pthread_mutex_lock(&room->mutex);
            }

            // Decrementa o timer
            room->timer--;

        } else {
            perror("select");
        }
    }

    // Iniciar jogo
    printf("Jogo na sala %d iniciado às %s\n", room->id, ctime(&room->startTime));

    pthread_mutex_unlock(&room->timerMutex);
}

// Função que envia atualizações do timer para o cliente, considerando o status premium
void sendTimerUpdate(ServerConfig *config, Room *room, Client *client) {

    // Preparar a mensagem de atualização do timer
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    sprintf(buffer, "TIMERUPDATE\n%d\n%d\n%d\n%d\n", 
            room->timer, room->id, room->game->id, room->numClients);

    // Enviar a mensagem de atualização
    if (send(client->socket_fd, buffer, strlen(buffer), 0) < 0) {
        // erro ao enviar mensagem
        err_dump(config, room->game->id, client->clientID, "can't send update to client", EVENT_MESSAGE_SERVER_NOT_SENT);
    } else {
        // Escrever no log a atualização enviada, considerando o status premium
        char logMessage[256];
        snprintf(logMessage, sizeof(logMessage), "%d %s: Time left: %d seconds - Room ID: %d - Game ID: %d - Clients joined: %d", 
                client->clientID, client->isPremium ? "(Premium User)" : "", room->timer, room->id, room->game->id, room->numClients);
        
        produceLog(config, logMessage, EVENT_MESSAGE_SERVER_SENT, room->game->id, client->clientID);
        printf("%s\n", logMessage);
    }
}
