#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include <string.h> // Usar strings (strcpy(), strcmp())
#include <time.h>   // Usar time()
#include <stdbool.h> // Usar bool
#include <pthread.h>
#include <unistd.h>
#include "../../utils/logs/logs-common.h"
#include "../../utils/parson/parson.h"
#include "server-game.h"
#include "../logs/logs.h"

static int nextRoomID = 1;

void saveRoomStatistics(int roomId, double elapsedTime) {
    FILE *file = fopen("room_stats.log", "a");  // Abre o ficheiro em modo de append
    if (file != NULL) {
        fprintf(file, "Sala %d - Tempo de resolução: %.2f segundos\n", roomId, elapsedTime);
        fclose(file);
    } else {
        printf("Erro ao abrir o ficheiro de estatísticas.\n");
    }
}

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

void updateGameStatistics(ServerConfig *config, int gameID, int elapsedTime, float accuracy) {

    FILE *file = fopen(config->gamePath, "r");

    // Ler o ficheiro JSON
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);
    if (file_content == NULL) {
        fclose(file);
        err_dump(config, gameID, 0, "memory allocation failed when updating statistics", MEMORY_ERROR);
        return;
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

    // iterate over the games array
    for (int i = 0; i < json_array_get_count(games_array); i++) {

        JSON_Object *game_object = json_array_get_object(games_array, i);
        int currentID = (int)json_object_get_number(game_object, "id");

        // if the game is found
        if (currentID == gameID) {

            int previousTimeRecord = (int)json_object_get_number(game_object, "timeRecord");
            float previousAccuracyRecord = (float)json_object_get_number(game_object, "accuracyRecord");
            
            // if elapsed time is less than the current one store it
            if (elapsedTime < previousTimeRecord || previousTimeRecord == 0) {
                json_object_set_number(game_object, "timeRecord", elapsedTime);

                // write to log
                char logMessage[100];
                snprintf(logMessage, sizeof(logMessage), "Tempo recorde atualizado para %d segundos no jogo %d", elapsedTime, gameID);
                produceLog(config, logMessage, EVENT_NEW_RECORD, gameID, 0);

                printf("Tempo recorde atualizado de %d para %d segundos no jogo %d\n", previousTimeRecord, elapsedTime, gameID);
            }

            // if accuracy is greater than the current one store it
            if (accuracy > previousAccuracyRecord || previousAccuracyRecord == 0) {
                json_object_set_number(game_object, "accuracyRecord", accuracy);

                // write to log
                char logMessage[100];
                snprintf(logMessage, sizeof(logMessage), "Precisão recorde atualizada para %f no jogo %d", accuracy, gameID);
                produceLog(config, logMessage, EVENT_NEW_RECORD, gameID, 0);

                printf("Precisão recorde atualizada de %.2f para %.2f no jogo %d\n", previousAccuracyRecord, accuracy, gameID);
            }
            break;
        }
    }

    // write the updated JSON to the file
    file = fopen(config->gamePath, "w");
    if (file == NULL) {
        err_dump(config, gameID, 0, "can't open file to write updated statistics", MEMORY_ERROR);
        json_value_free(root_value);
        return;
    }

    char *serialized_string = json_serialize_to_string_pretty(root_value);
    fwrite(serialized_string, 1, strlen(serialized_string), file);
    fclose(file);
    free(serialized_string);

    // free the JSON object
    json_value_free(root_value);
}

void acquireReadLock(Room *room) {
    // indicate that a reader is entering the room
    sem_wait(&room->readSemaphore);

    // lock mutex
    pthread_mutex_lock(&room->readMutex);

    // increment reader count
    room->readerCount++;

    // if first reader lock the write semaphore
    if (room->readerCount == 1) {
        sem_wait(&room->writeSemaphore); //(decrementa o semáforo)
    }

    // unlock mutex
    pthread_mutex_unlock(&room->readMutex);

    // unlock the reader semaphore
    sem_post(&room->readSemaphore);
}

void releaseReadLock(Room *room) {
    // lock mutex
    pthread_mutex_lock(&room->readMutex);

    // decrement reader count
    room->readerCount--;

    // if last reader unlock the write semaphore
    if (room->readerCount == 0) {
        sem_post(&room->writeSemaphore);//(incrementa o semáforo)
    }

    // unlock mutex
    pthread_mutex_unlock(&room->readMutex);
}

void acquireWriteLock(Room *room, Client *client) {

    // add a random delay to appear more natural
    //int delay = rand() % 5;
    //sleep(delay);

    pthread_mutex_lock(&room->writeMutex);

    // increment writer count
    room->writerCount++;

    printf("WRITER COUNT: %d from client %d\n", room->writerCount, client->clientID);

    // if first writer lock the read semaphore to block readers
    if (room->writerCount == 1) {
        printf("WRITER %d IS LOCKING READ SEMAPHORE\n", client->clientID);
        sem_wait(&room->readSemaphore);
    }

    // unlock the writer mutex
    pthread_mutex_unlock(&room->writeMutex);

    // lock the write semaphore
    //printf("WRITER %d IS WAITING FOR WRITE SEMAPHORE\n", client->clientID);
    sem_wait(&room->writeSemaphore);
}

void releaseWriteLock(Room *room, Client *client) {

    sem_post(&room->writeSemaphore);

    // lock the writer mutex
    pthread_mutex_lock(&room->writeMutex);

    // decrement writer count
    room->writerCount--;

    // if last writer unlock the read semaphore
    if (room->writerCount == 0) {
        sem_post(&room->readSemaphore);
    }

    //printf("WRITER %d IS DONE WRITING\n", client->clientID);

    // unlock the writer mutex
    pthread_mutex_unlock(&room->writeMutex);

}

void acquireTurnsTileSemaphore(Room *room, Client *client) {

    // lock the mutex
    sem_wait(&room->mutexSemaphore);

    //printf("CLIENT %d ARRIVED AT THE TILE SEMAPHORE\n", client->clientID);

    // increment the waiting count
    room->waitingCount++;

    // if all players are waiting unlock the turns tile semaphore
    if (room->waitingCount == room->numClients) {

        //printf("CLIENT %d ARRIVED AND IT'S THE LAST ONE\n", client->clientID);
        //printf("RELEASING TURNS TILE SEMAPHORE\n");

        // need to loop n times to unlock the turns tile semaphore
        for (int i = 0; i < room->numClients; i++) {
            sem_post(&room->turnsTileSemaphore1);
        }
    }

    // unlock the mutex
    sem_post(&room->mutexSemaphore);

    //printf("CLIENT %d IS WAITING FOR TURNS TILE SEMAPHORE\n", client->clientID);
    // wait for the turns tile semaphore
    sem_wait(&room->turnsTileSemaphore1);

    //printf("CLIENT %d IS DONE WAITING FOR TURNS TILE SEMAPHORE\n", client->clientID);
}

void releaseTurnsTileSemaphore(Room *room, Client *client) {

    // lock the mutex
    sem_wait(&room->mutexSemaphore);

    printf("CLIENT %d ARRIVED AT THE TILE SEMAPHORE\n", client->clientID);

    // decrement the waiting count
    room->waitingCount--;

    // if all players are done unlock the turns tile semaphore
    if (room->waitingCount == 0) {

        printf("CLIENT %d ARRIVED AND IT'S THE LAST ONE\n", client->clientID);
        printf("RELEASING TURNS TILE SEMAPHORE\n");

        // need to loop n times to unlock the turns tile semaphore
        for (int i = 0; i < room->maxClients; i++) {
            sem_post(&room->turnsTileSemaphore2);
        }
    }

    // unlock the mutex
    sem_post(&room->mutexSemaphore);

    printf("CLIENT %d IS WAITING FOR TURNS TILE SEMAPHORE\n", client->clientID);
    // wait for the turns tile semaphore
    sem_wait(&room->turnsTileSemaphore2);
    printf("CLIENT %d IS DONE WAITING FOR TURNS TILE SEMAPHORE\n", client->clientID);

}

void enterBarberShop(Room *room, Client *client) {

    // initialize self semaphore
    sem_init(&client->selfSemaphore, 0, 0);

    // lock the barber shop mutex
    pthread_mutex_lock(&room->barberShopMutex);

    // increment the number of customers
    room->customers++;

    //printf("CLIENT %d ENTERED THE BARBER SHOP\n", client->clientID);
    // add the client to the barber shop queue
    if (room->priorityQueueType == 0) { // static priority
        enqueueWithPriority(room->barberShopQueue, client->clientID, client->isPremium);
    } else if (room->priorityQueueType == 1) { // dynamic priority
        enqueueWithPriority(room->barberShopQueue, client->clientID, client->isPremium);
        updateQueueWithPriority(room->barberShopQueue, room->maxWaitingTime);
    } else { // FIFO
        enqueueFifo(room->barberShopQueue, client->clientID);
    }
        
    // unlock the barber shop mutex
    pthread_mutex_unlock(&room->barberShopMutex);

    //printf("CLIENT %d IS WAITING FOR THE BARBER1\n", client->clientID);

    // wait for the costumer semaphore
    sem_post(&room->costumerSemaphore);

    //printf("CLIENT %d IS WAITING FOR THE BARBER2\n", client->clientID);

    // wait for the self semaphore to be unlocked by the dequeue function
    sem_wait(&client->selfSemaphore);
}

void leaveBarberShop(Room *room, Client *client) {

    //printf("CLIENT %d IS DONE WITH THE BARBER\n", client->clientID);

    // signal that the costumer is done
    sem_post(&room->costumerDoneSemaphore);

    //printf("CLIENT %d IS WAITING FOR THE BARBER TO BE DONE\n", client->clientID);

    // wait for the barber to be done
    sem_wait(&room->barberDoneSemaphore);

    // lock the barber shop mutex
    pthread_mutex_lock(&room->barberShopMutex);

    // decrement the number of customers
    room->customers--;

    // unlock the barber shop mutex
    pthread_mutex_unlock(&room->barberShopMutex);
}

void barberCut(Room *room) {

    //printf("BARBER IS WAITING FOR A COSTUMER\n");

    

    // wait for the costumer semaphore
    sem_wait(&room->costumerSemaphore);

    // lock the barber shop mutex
    pthread_mutex_lock(&room->barberShopMutex);

    //printf("NUMBER OF CUSTOMERS: %d\n", room->customers);

    // dequeue the client from the barber shop queue
    int clientID = dequeue(room->barberShopQueue);
    Client *client;

    if (clientID == -1) {
        printf("NO CLIENTS IN THE BARBER SHOP\n");
        // unlock the barber shop mutex
        pthread_mutex_unlock(&room->barberShopMutex);
        return;
    }

    // unlock the self semaphore
    for (int i = 0; i < room->maxClients; i++) {
        if (room->clients[i]->clientID == clientID) {
            client = room->clients[i];
            break;
        }
    }

    // unlock the barber shop mutex
    pthread_mutex_unlock(&room->barberShopMutex);

    // post the self semaphore
    sem_post(&client->selfSemaphore);

    // delete the self semaphore
    sem_destroy(&client->selfSemaphore);
}

void barberIsDone(Room *room) {

    //printf("BARBER IS DONE WITH THE COSTUMER1\n");
    // wait for the costumer done semaphore
    sem_wait(&room->costumerDoneSemaphore);

    //printf("BARBER IS DONE WITH THE COSTUMER2\n");

    // signal that the barber is done
    sem_post(&room->barberDoneSemaphore);
}

void *handleBarber(void *arg) {

    while (1) {

        Room *room = (Room *)arg;

        barberCut(room);
        barberIsDone(room);
    }
}