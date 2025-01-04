#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include <string.h> // Usar strings (strcpy(), strcmp())
#include <time.h>   // Usar time()
#include <stdbool.h> // Usar bool
#include <pthread.h>
#include <unistd.h>
#include "../../utils/logs/logs-common.h"
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "server-game.h"
#include "../logs/logs.h"

static int nextRoomID = 1;

/**
 * Gera um identificador único para uma nova sala de jogo.
 *
 * @return Um identificador único para a sala, incrementando o valor de `nextRoomID`.
 *
 * @details Esta função utiliza uma variável estática `nextRoomID` para gerar IDs únicos 
 * e sequenciais para as salas de jogo.
 * Cada vez que a função é chamada, o valor de `nextRoomID` é retornado e, em seguida, incrementado,
 * garantindo que cada sala criada tenha um ID exclusivo.
 */

int generateUniqueId() {
    return nextRoomID++;
}

/**
 * Carrega um jogo a partir do ficheiro 'games.json' com base no ID do jogo.
 *
 * @param config Um pointer para a estrutura `ServerConfig` que contém o caminho 
 * para o ficheiro 'games.json' e o ficheiro de log.
 * @param gameID O identificador do jogo que se pretende carregar.
 * @param playerID O identificador do jogador que está a tentar carregar o jogo, 
 * usado para o registo no log.
 * @return Um pointer para a estrutura `Game` carregada, ou NULL se o jogo não for encontrado ou ocorrer um erro.
 *
 * @details Esta função faz o seguinte:
 * - Aloca memória para a estrutura `Game` e inicializa-a a zeros.
 * - Abre o ficheiro 'games.json' e lê o seu conteúdo. Se ocorrer um erro, regista-o no log e devolve NULL.
 * - Faz o parse do conteúdo JSON e percorre o array de jogos para encontrar o jogo com o ID correspondente.
 * - Se o jogo for encontrado, preenche o tabuleiro (`board`) e a solução (`solution`) na estrutura `Game`.
 * - Regista o carregamento bem-sucedido no log e devolve o pointer para o jogo.
 * - Se o jogo não for encontrado, regista o erro no log, imprime uma mensagem de erro no terminal, e devolve NULL.
 * - Liberta a memória alocada para o conteúdo JSON e a estrutura `Game` em caso de falha.
 */

Game *loadGame(ServerConfig *config, int gameID, int playerID) {

    Game *game = (Game *)malloc(sizeof(Game));
    if (game == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    memset(game, 0, sizeof(Game));  // Initialize game struct

    FILE *file = fopen(config->gamePath, "r");

    if (file == NULL) {
        produceLog(config, "Erro ao abrir o ficheiro de jogos.", EVENT_GAME_NOT_LOAD, gameID, playerID);
        free(game);  // Free allocated memory before exiting
        return NULL;
    }

    // Ler o ficheiro JSON
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);
    if (file_content == NULL) {
        fclose(file);
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    fread(file_content, 1, file_size, file);
    file_content[file_size] = '\0';
    fclose(file);

    // Parse do JSON
    JSON_Value *root_value = json_parse_string(file_content);
    JSON_Object *root_object = json_value_get_object(root_value);
    free(file_content);

    // Get the "games" array
    JSON_Array *games_array = json_object_get_array(root_object, "games");

    // Iterate over the games array to find the matching ID
    for (int i = 0; i < json_array_get_count(games_array); i++) {

        // get json object
        JSON_Object *game_object = json_array_get_object(games_array, i);

        // get current id from Json object
        int currentID = (int)json_object_get_number(game_object, "id");
        
        // se o game for encontrado
        if (currentID == gameID) {

            // set game id as currentID
            game->id = currentID;

            // set current line as 1
            game->currentLine = 1;

             // Obter o board e a solution do JSON
            for (int row = 0; row < 9; row++) {

                JSON_Array *board_row = json_array_get_array(json_object_get_array(game_object, "board"), row);
                JSON_Array *solution_row = json_array_get_array(json_object_get_array(game_object, "solution"), row);

                for (int col = 0; col < 9; col++) {
                    game->board[row][col] = (int)json_array_get_number(board_row, col);
                    game->solution[row][col] = (int)json_array_get_number(solution_row, col);
                }
            }
      
            // game has been loaded successfully
            produceLog(config, "Jogo carregado com sucesso", EVENT_GAME_LOAD, gameID, playerID);

            // if game has been found leave loop
            json_value_free(root_value);
            return game; // Return the game immediately
        } 
    }

    // Game not found
    if (game->id == 0) {
        
        // cria mensagem mais detalhada
        char logMessage[100];
        snprintf(logMessage, sizeof(logMessage), "game com ID %d nao encontrado", gameID);
        produceLog(config, logMessage, EVENT_GAME_NOT_FOUND, gameID, playerID);

        // mostra no terminal e encerra programa
        fprintf(stderr, "game com ID %d nao encontrado.\n", gameID);
        free(game);
        return NULL;
    }

    // Limpar memoria alocada ao JSON
    json_value_free(root_value);

    return game;
}


/**
 * Carrega um jogo aleatório a partir do ficheiro 'games.json'.
 *
 * @param config Um pointer para a estrutura `ServerConfig` que contém o caminho 
 * para o ficheiro 'games.json' e o caminho do log.
 * @param playerID O identificador do jogador que está a solicitar um jogo aleatório, 
 * usado para o registo no log.
 * @return Um pointer para a estrutura `Game` carregada, ou NULL se ocorrer um erro.
 *
 * @details Esta função faz o seguinte:
 * - Abre o ficheiro 'games.json' para leitura. Se o ficheiro não puder ser aberto, 
 * regista um erro no log e termina o programa.
 * - Lê o conteúdo do ficheiro e garante que a string termina com um caractere nulo.
 * - Faz o parse do conteúdo JSON e obtém o array de jogos.
 * - Usa uma semente baseada no tempo atual para gerar um ID de jogo aleatório.
 * - Chama a função `loadGame` para carregar o jogo aleatório selecionado.
 * - Liberta a memória alocada para o conteúdo JSON e retorna o jogo carregado.
 */

Game *loadRandomGame(ServerConfig *config, int playerID) {

    // open the file
    FILE *file = fopen(config->gamePath, "r");

    // check if file is opened
    if (file == NULL) {
        err_dump(config, 0, playerID, "Erro ao abrir o ficheiro de jogos.", EVENT_GAME_NOT_LOAD);
    }

    // get the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read the file content
    char *file_content = malloc(file_size + 1);
    fread(file_content, 1, file_size, file);

    // add null terminator to the end of the file content
    file_content[file_size] = '\0';

    // close the file
    fclose(file);

    // parse the JSON
    JSON_Value *root_value = json_parse_string(file_content);
    JSON_Object *root_object = json_value_get_object(root_value);

    // free the file content
    free(file_content);

    // get the games array
    JSON_Array *games_array = json_object_get_array(root_object, "games");

    // get the number of games
    int numberOfGames = json_array_get_count(games_array);

    // get seed for random number generator
    srand(time(NULL));

    // get a random game ID
    int randomGameID = rand() % numberOfGames + 1;
    printf("Random game ID selected: %d from %d games\n", randomGameID, numberOfGames);
    
    // free the JSON object
    json_value_free(root_value);

    // return the loaded game
    return loadGame(config, randomGameID, playerID);
}


/**
 * Verifica se uma linha inserida pelo jogador está correta em relação à solução do jogo.
 *
 * @param logFileName O caminho para o ficheiro de log onde os eventos são registados.
 * @param solutionSent Uma string que representa a solução enviada pelo jogador.
 * @param game Um pointer para a estrutura `Game` que contém o estado atual do tabuleiro e a solução correta.
 * @param insertLine Um array de 9 inteiros que representa a linha inserida pelo jogador.
 * @param lineNumber O número da linha a ser verificada (0-indexado).
 * @param playerID O identificador do jogador que enviou a linha.
 * @return 1 se a linha estiver correta, 0 se estiver incorreta ou incompleta.
 *
 * @details Esta função faz o seguinte:
 * - Regista no log a solução enviada pelo jogador, incluindo o ID do jogador, o ID do jogo, e o número da linha.
 * - Itera sobre os 9 valores da linha inserida e compara cada valor com a solução do jogo.
 * - Se o valor estiver correto, atualiza o tabuleiro do jogo com o valor inserido.
 * - Imprime no terminal a posição da linha, o valor esperado, e o valor recebido para facilitar o debug.
 * - Verifica se a linha está correta usando a função `isLineCorrect`.
 * - Regista no log se a linha foi validada como correta ou incorreta e devolve 1 ou 0, respetivamente.
 */

int verifyLine(ServerConfig *config, Game *game, char * solutionSent, int insertLine[9], int playerID) {

    char logMessage[100];
    
    sprintf(logMessage, "O jogador %d no jogo %d para a linha %d: %s", playerID, game->id, game->currentLine, solutionSent);
    produceLog(config, logMessage, EVENT_SOLUTION_SENT, game->id, playerID);
    
    for (int j = 0; j < 9; j++) {

        // verifica se o valor inserido é igual ao valor da solução
        if (insertLine[j] == game->solution[game->currentLine - 1][j]) {
            
            // se for igual, atualiza o tabuleiro
            game->board[game->currentLine - 1][j] = insertLine[j];

        // se o valor inserido for diferente do valor da solução
        } 

        //printf("Posição %d: esperado %d, recebido %d\n", j + 1, game->solution[game->currentLine - 1][j], insertLine[j]);
    }

    // limpa logMessage
    memset(logMessage, 0, sizeof(logMessage));

    // verifica se a linha está correta
    if (isLineCorrect(game, game->currentLine - 1)) {

        // linha correta
        snprintf(logMessage, sizeof(logMessage), "Linha enviada (%s) validada como CERTA", solutionSent);
        produceLog(config, logMessage, EVENT_SOLUTION_CORRECT, game->id, playerID);
        return 1;

    } else {

        // linha incorreta
        snprintf(logMessage, sizeof(logMessage), "Linha enviada (%s) validada como ERRADA/INCOMPLETA", solutionSent);
        produceLog(config, logMessage, EVENT_SOLUTION_INCORRECT, game->id, playerID);
        return 0;
    }
}

/**
 * Verifica se uma linha específica do tabuleiro do jogo está correta em relação à solução.
 *
 * @param game Um pointer para a estrutura `Game` que contém o tabuleiro atual e a solução correta.
 * @param row O número da linha (0-indexado) que deve ser verificada.
 * @return `true` se todos os valores da linha estiverem corretos, `false` caso contrário.
 *
 * @details Esta função percorre os 9 valores da linha especificada no tabuleiro do jogo.
 * Se algum valor da linha atual não corresponder ao valor na solução, a função devolve `false`.
 * Se todos os valores forem iguais, a função devolve `true`, indicando que a linha está correta.
 */

bool isLineCorrect(Game *game, int row) {
    for (int i = 0; i < 9; i++) {
        if (game->board[row][i] != game->solution[row][i]) {
            return false;
        }
    }
    return true;
}


/**
 * Cria uma nova sala de jogo e inicializa os seus campos.
 *
 * @param config Um pointer para a estrutura `ServerConfig` que contém as configurações do servidor, 
 * incluindo o número máximo de salas e o caminho para o ficheiro de log.
 * @return Um pointer para a nova estrutura `Room` criada, ou NULL se a alocação de memória falhar.
 *
 * @details Esta função faz o seguinte:
 * - Aloca memória para uma nova estrutura `Room` e inicializa os seus campos a zeros.
 * - Gera um identificador único para a sala usando `generateUniqueId`.
 * - Aloca memória para o array de jogadores da sala, com o tamanho máximo definido em `config`.
 * - Inicializa o array `config->rooms` a zeros para garantir que todos os campos estão corretamente definidos.
 * - Incrementa o número de salas ativas no servidor.
 * - Regista a criação da sala no ficheiro de log.
 * - Devolve o pointer para a sala criada, ou NULL se a alocação de memória falhar.
 */

Room *createRoom(ServerConfig *config, int playerID, bool isSinglePlayer, int synchronizationType) {

    Room *room = (Room *)malloc(sizeof(Room));
    if (room == NULL) {
        err_dump(config, 0, playerID, "Memory allocation failed", EVENT_ROOM_NOT_LOAD);
        return NULL;
    }
    memset(room, 0, sizeof(Room));  // Initialize Room struct

    room->id = generateUniqueId();
    printf("Room ID na criação da room: %d\n", room->id);
    room->timer = 60;
    room->isGameRunning = false;
    room->isFinished = false;
    room->isSinglePlayer = isSinglePlayer;
    room->maxClients = room->isSinglePlayer ? 1 : config->maxClientsPerRoom;
    room->clients = (Client **)malloc(sizeof(Client *) * room->maxClients);
    room->maxWaitingTime = config->maxWaitingTime;

    // we dont need synchronization for single player games
    if (!room->isSinglePlayer) {

        // Inicializar priority queue
        room->enterRoomQueue = (PriorityQueue *)malloc(sizeof(PriorityQueue));
        initPriorityQueue(room->enterRoomQueue, room->maxClients * 2);

        // Initialize reader-writer locks
        sem_init(&room->writeSemaphore, 0, 1); // Inicializar semáforo para escrita e começa a aceitar 1 escritor
        sem_init(&room->readSemaphore, 0, 1); // Inicializar semáforo para leitura e começa a aceitar 1 leitor
        sem_init(&room->nonPremiumWriteSemaphore, 0, 0); //
        pthread_mutex_init(&room->readMutex, NULL); // Inicializar mutex para leitura
        pthread_mutex_init(&room->writeMutex, NULL); // Inicializar mutex para escrita
        room->readerCount = 0;
        room->writerCount = 0;

        // Inicializar barreira para começar e terminar o jogo
        room->waitingCount = 0;
        sem_init(&room->mutexSemaphore, 0, 1);
        sem_init(&room->turnsTileSemaphore1, 0, 0);
        sem_init(&room->turnsTileSemaphore2, 0, 0);

        // barber shop initialization
        room->customers = 0;
        pthread_mutex_init(&room->barberShopMutex, NULL);
        sem_init(&room->costumerSemaphore, 0, 0);
        sem_init(&room->costumerDoneSemaphore, 0, 0);
        sem_init(&room->barberDoneSemaphore, 0, 0);
        room->barberShopQueue = (PriorityQueue *)malloc(sizeof(PriorityQueue));
        initPriorityQueue(room->barberShopQueue, room->maxClients);

        // check if the game is reader-writer or barber shop
        if (synchronizationType == 0) {
            room->isReaderWriter = true;
        }

        if (synchronizationType == 1) { // barber shop with static priority
            room->isReaderWriter = false;
            room->priorityQueueType = 0;
        }

        if (synchronizationType == 2) { // barber shop with dynamic priority
            room->isReaderWriter = false;
            room->priorityQueueType = 1;
        }

        if (synchronizationType == 3) { // barber shop with FIFO
            room->isReaderWriter = false;
            room->priorityQueueType = 2;
        }

        if (!room->isReaderWriter) {
            // initialize thread for barber
            if(pthread_create(&room->barberThread, NULL, handleBarber, (void *)room)) {
                err_dump(config, 0, playerID, "Erro ao criar a thread do barbeiro", EVENT_THREAD_NOT_CREATE);
            }
            produceLog(config, "Barbeiro criado com sucesso", EVENT_BARBER_CREATED, room->id, playerID);
        }
        
        // initialize mutexes
        pthread_mutex_init(&room->mutex, NULL);
        pthread_mutex_init(&room->timerMutex, NULL);
    }

    // add the room to the list of rooms and increment the number of rooms
    config->rooms[config->numRooms++] = room;

    // log room creation
    produceLog(config, "Sala criada com sucesso", EVENT_ROOM_LOAD, room->id, playerID);
    return room;
}

Room *getRoom(ServerConfig *config, int roomID, int playerID) {

    // check if roomID is valid
    if (roomID < 1) {
        err_dump(config, 0, playerID, "Room ID is invalid", EVENT_ROOM_NOT_JOIN);
        return NULL;
    }

    // get the room by looping through the rooms
    Room *room = NULL;
    for (int i = 0; i < config->numRooms; i++) {
        //printf("i: %d\n", i);
        //printf("NR rooms: %d\n", config->numRooms);
        //printf("ROOM ID: %d\n", config->rooms[i]->id);
        if (config->rooms[i]->id == roomID) {
            room = config->rooms[i];
            break;
        }
    }

    return room;
}

void deleteRoom(ServerConfig *config, int roomID) {
    for (int i = 0; i < config->numRooms; i++) {
        if (config->rooms[i]->id == roomID) {
            printf("FREEING MEMORY FOR ROOM %d\n", roomID);
            
            free(config->rooms[i]->clients); 

            free(config->rooms[i]->game);

            // Destruir mutexes e semáforos
            if (!config->rooms[i]->isSinglePlayer) {
                pthread_mutex_destroy(&config->rooms[i]->mutex);
                pthread_mutex_destroy(&config->rooms[i]->timerMutex);
                pthread_mutex_destroy(&config->rooms[i]->readMutex);
                pthread_mutex_destroy(&config->rooms[i]->writeMutex);
                pthread_mutex_destroy(&config->rooms[i]->barberShopMutex);
                sem_destroy(&config->rooms[i]->mutexSemaphore);
                sem_destroy(&config->rooms[i]->turnsTileSemaphore1);
                sem_destroy(&config->rooms[i]->turnsTileSemaphore2);
                sem_destroy(&config->rooms[i]->writeSemaphore);
                sem_destroy(&config->rooms[i]->readSemaphore);
                sem_destroy(&config->rooms[i]->nonPremiumWriteSemaphore);
                sem_destroy(&config->rooms[i]->costumerSemaphore);
                sem_destroy(&config->rooms[i]->costumerDoneSemaphore);
                sem_destroy(&config->rooms[i]->barberDoneSemaphore);

                // destroy barber
                if (!config->rooms[i]->isReaderWriter) {
                    pthread_cancel(config->rooms[i]->barberThread);
                }

                // free priority queue
                freePriorityQueue(config->rooms[i]->enterRoomQueue);
                freePriorityQueue(config->rooms[i]->barberShopQueue);
            }

            free(config->rooms[i]);

            // shift all rooms to the left
            for (int j = i; j < config->numRooms - 1; j++) {
                config->rooms[j] = config->rooms[j + 1];
            }

            // decrement number of rooms
            config->numRooms--;

            // log room deletion
            produceLog(config, "Sala eliminada com sucesso", EVENT_ROOM_DELETE, roomID, 0);

            break;
        }
    }

    
}

/**
 * Obtém uma lista de identificadores de jogos a partir do ficheiro 'games.json'.
 *
 * @param config Um pointer para a estrutura `ServerConfig` que contém 
 * o caminho para o ficheiro 'games.json' e o caminho do log.
 * @return Uma string que contém os IDs de todos os jogos, formatados com 
 * "Game ID: [ID]", ou NULL se ocorrer um erro na leitura do ficheiro.
 *
 * @details Esta função faz o seguinte:
 * - Abre o ficheiro 'games.json' e lê todo o seu conteúdo. Se o ficheiro não puder ser aberto,
 * regista o erro no log e termina o programa.
 * - Faz o parse do conteúdo JSON para obter o array de jogos.
 * - Itera sobre cada jogo no array e extrai o ID do jogo.
 * - Concatena todos os IDs numa string, cada um formatado como "Game ID: [ID]".
 * - Liberta a memória alocada para o conteúdo JSON e devolve a string que contém os IDs.
 * 
 * @note O tamanho da string `games` é limitado a 1024 caracteres, o que pode ser ajustado 
 * conforme necessário para evitar overflow.
 */

char *getGames(ServerConfig *config) {

    // open the file
    FILE *file = fopen(config->gamePath, "r");

    // check if file is opened
    if (file == NULL) {
        err_dump(config, 0, 0, "Erro ao abrir o ficheiro de jogos.", EVENT_GAME_NOT_LOAD);
    }

    // get the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read the file content
    char *file_content = malloc(file_size + 1);
    fread(file_content, 1, file_size, file);

    // add null terminator to the end of the file content
    file_content[file_size] = '\0';

    // close the file
    fclose(file);

    // get ids of the games
    JSON_Value *root_value = json_parse_string(file_content);
    JSON_Object *root_object = json_value_get_object(root_value);

    // free the file content
    free(file_content);

    // get the games array
    JSON_Array *games_array = json_object_get_array(root_object, "games");

    // get the number of games
    int numberOfGames = json_array_get_count(games_array);

    // create a string to store the ids of the games
    char *games = (char *)malloc(1024);
    memset(games, 0, 1024);

    // iterate over the games array
    for (int i = 0; i < numberOfGames; i++) {
        JSON_Object *game_object = json_array_get_object(games_array, i);
        int gameID = (int)json_object_get_number(game_object, "id");
        char gameIDString[10];
        sprintf(gameIDString, "%d", gameID);
        strcat(games, "Game ID: ");
        strcat(games, gameIDString);
        strcat(games, "\n");
    }

    // free the JSON object
    json_value_free(root_value);

    // return the games
    return games;
}

/**
 * Obtém uma lista de salas de jogo ativas no servidor, incluindo o ID da sala, o número de jogadores, 
 * o número máximo de jogadores, e o ID do jogo associado.
 *
 * @param config Um pointer para a estrutura `ServerConfig` que contém as informações sobre as salas de jogo.
 * @return Uma string que contém a lista de salas de jogo formatada, 
 * ou uma mensagem "No rooms available\n" se não houver salas ativas.
 *
 * @details Esta função faz o seguinte:
 * - Verifica se há salas disponíveis. Se o número de salas for zero, devolve a mensagem "No rooms available\n".
 * - Cria uma string para armazenar as informações de cada sala, incluindo o ID da sala, o número de jogadores atuais, 
 *   o número máximo de jogadores, e o ID do jogo.
 * - Itera sobre todas as salas e concatena as informações formatadas de cada sala na string `rooms`.
 * - Devolve a string `rooms` contendo as informações de todas as salas. A memória alocada para esta 
 * string deve ser libertada pelo chamador.
 *
 * @note O tamanho da string `rooms` é limitado a 1024 caracteres, o que pode ser ajustado
 * se houver um número elevado de salas.
 */

char *getRooms(ServerConfig *config) {

        // create a string to store the rooms
        char *rooms = (char *)malloc(BUFFER_SIZE);
        memset(rooms, 0, 1024);

        // iterate over the rooms
        if (config->numRooms == 0) {
            strcpy(rooms, "No rooms available\n");
        }

        for (int i = 0; i < config->numRooms; i++) {
            if (config->rooms[i] == NULL) {
                continue;
            }
            // can only join rooms that are not running
            if (config->rooms[i]->isGameRunning == false) {
                // show number of players in the room, max players in the room and the game ID
                char roomString[100];
                sprintf(roomString, "Room ID: %d, Players: %d/%d, Game ID: %d\n", config->rooms[i]->id, config->rooms[i]->numClients, config->rooms[i]->maxClients, config->rooms[i]->game->id);
                strcat(rooms, roomString);
            }
        }

        // if no rooms are available
        if (strlen(rooms) == 0) {
            strcpy(rooms, "No rooms available\n");
        }

        // return the rooms
        return rooms;
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
        printf("Waiting for more Clients to join in room %d\n", room->id);
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

    //printf("Client %d joined room %d\n", client->clientID, room->id);

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

    //printf("---------------------------------------------\n");
    //printf("INÍCIO DA SECÇÃO CRÍTICA DE LEITURA para o cliente %d na sala %d\n", client->clientID, room->id);

    int correctLine = 0;

    // critical section reader
    sendBoard(config, room, client);

    //printf("FIM DA SECÇÃO CRÍTICA DE LEITURA para o cliente %d na sala %d\n", client->clientID, room->id);
    //printf("---------------------------------------------\n");

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

            printf("Cliente %d %s quer resolver a linha %d na sala %d com o jogo %d\n", 
            client->clientID, client->isPremium ? "(PREMIUM)" : "(NOT PREMIUM)",
            room->id, room->game->id, room->game->currentLine);

            // pre condition writer
            if (!room->isSinglePlayer) {
                if (room->isReaderWriter) {
                    acquireWriteLock(room, client);
                } else {
                    enterBarberShop(room, client);
                }
            }

            //printf("-----------------------------------------------------\n");
            //printf("INÍCIO DA SECÇÃO CRÍTICA DE ESCRITA para o cliente %d na sala %d\n", client->clientID, room->id);

            // **Adicionei print para verificar a linha recebida**
            //printf("Linha recebida do cliente %d: %s\n", client->clientID, line); 

            // Converte a linha recebida em valores inteiros
            int insertLine[9];
            for (int j = 0; j < 9; j++) {
                insertLine[j] = line[j] - '0';
            }

            printf("Verificando linha %d do cliente %d na sala %d com o o jogo %d\n", 
            room->game->currentLine, client->clientID, room->id, room->game->id);
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

            //printf("FIM DA SECÇÃO CRÍTICA DE ESCRITA para o cliente %d na sala %d\n", client->clientID, room->id);
            //printf("-----------------------------------------------------\n");

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
                    //printf("Sent timer update to Client %d on socket %d\n", room->clients[i]->clientID, room->clients[i]->socket_fd);
                    
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
        snprintf(logMessage, sizeof(logMessage), "Sent update to Client %d %s - Time left: %d seconds - Room ID: %d - Game ID: %d - Clients joined: %d", 
                client->clientID, client->isPremium ? "(Premium User)" : "(Non Premium User)", room->timer, room->id, room->game->id, room->numClients);
        
        produceLog(config, logMessage, EVENT_MESSAGE_SERVER_SENT, room->game->id, client->clientID);
        printf("%s\n", logMessage);
    }
}
