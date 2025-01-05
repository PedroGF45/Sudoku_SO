#include <pthread.h>
#include <sys/select.h>
#include <time.h>
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../../utils/logs/logs-common.h"
#include "server-comms.h"
#include "../logs/logs.h"


static int nextClientID = 1;


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

        //printf("A aguardar por pedidos do cliente %d\n", client->clientID);

        client->startAgain = false;
        memset(buffer, 0, sizeof(buffer));

        // Receber menu status
        if (recv(newSockfd, buffer, sizeof(buffer), 0) < 0) {
            // erro ao receber modo de jogo
            err_dump(serverConfig, 0, client->clientID, "can't receive menu status", EVENT_MESSAGE_SERVER_NOT_RECEIVED);
        } else {

            //printf("BUFFER RECEBIDO: %s\n", buffer);
            
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

                //printf("Jogos existentes: %s\n", games);

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
                                //printf("BUFFER RECEBIDO PARA SYNCHRONIZATION TYPE: %s\n", buffer);
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

                //printf("Rooms existentes: %s\n", rooms);

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
                    //printf("RECEBER IDS DAS ROOMS\n");
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
                        //printf("ENQUEING CLIENT %d which is %s\n", client->clientID, client->isPremium ? "premium" : "not premium");
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
                                //printf("CHECKING IF ROOM IS FULL FOR CLIENT %d\n", clientTemp->clientID);
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
