#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include <string.h> // Usar strings (strcpy(), strcmp())
#include <time.h>   // Usar time()
#include <stdbool.h> // Usar bool
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"
#include "server-game.h"

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
        writeLogJSON(config->logPath, gameID, playerID, EVENT_GAME_NOT_LOAD);
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
      
            // game has been loaded
            writeLogJSON(config->logPath, gameID, playerID, EVENT_GAME_LOAD);

            // if game has been found leave loop
            json_value_free(root_value);
            return game; // Return the game immediately
        } 
    }

    // Game not found
    if (game->id == 0) {
        
        // cria mensagem mais detalhada
        char logMessage[100];
        snprintf(logMessage, sizeof(logMessage), "%s: game com ID %d nao encontrado", EVENT_GAME_NOT_LOAD, gameID);

        // escreve log de game nao encontrado
        writeLogJSON(config->logPath, gameID, playerID, logMessage);

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
        writeLogJSON(config->logPath, 0, playerID, EVENT_GAME_NOT_LOAD);
        exit(1);
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

int verifyLine(char * logFileName, char * solutionSent, Game *game, int insertLine[9], int playerID) {

    char logMessage[100];
    
    sprintf(logMessage, "O jogador %d no jogo %d %s para a linha %d: %s", playerID, game->id, EVENT_SOLUTION_SENT, game->currentLine, solutionSent);
    writeLogJSON(logFileName, game->id, playerID, logMessage);
    
    for (int j = 0; j < 9; j++) {

        // verifica se o valor inserido é igual ao valor da solução
        if (insertLine[j] == game->solution[game->currentLine - 1][j]) {
            
            // se for igual, atualiza o tabuleiro
            game->board[game->currentLine - 1][j] = insertLine[j];

        // se o valor inserido for diferente do valor da solução
        } 

        printf("Posição %d: esperado %d, recebido %d\n", j + 1, game->solution[game->currentLine - 1][j], insertLine[j]);
    }

    // limpa logMessage
    memset(logMessage, 0, sizeof(logMessage));

    // verifica se a linha está correta
    if (isLineCorrect(game, game->currentLine - 1)) {

        // linha correta
        snprintf(logMessage, sizeof(logMessage), "Linha enviada (%s) validada como CERTA", solutionSent);
        writeLogJSON(logFileName, game->id, playerID, logMessage);
        return 1;

    } else {

        // linha incorreta
        snprintf(logMessage, sizeof(logMessage), "Linha enviada (%s) validada como ERRADA/INCOMPLETA", solutionSent);
        writeLogJSON(logFileName, game->id, playerID, logMessage);
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

Room *createRoom(ServerConfig *config, int playerID) {

    Room *room = (Room *)malloc(sizeof(Room));
    if (room == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    memset(room, 0, sizeof(Room));  // Initialize Room struct

    room->id = generateUniqueId();
    room->players = (int *)malloc(config->maxPlayersPerRoom * sizeof(int));
    room->clientSockets = (int *)malloc(config->maxPlayersPerRoom * sizeof(int));
    room->timer = 60;
    room->isGameRunning = false;
    memset(config->rooms, 0, sizeof(Room) * config->maxRooms); // Ensures all fields in rooms are set to 0

    // increase the number of rooms
    config->numRooms++;
    
    writeLogJSON(config->logPath, 0, playerID, EVENT_ROOM_LOAD);
    return room;
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
        writeLogJSON(config->logPath, 0, 0, EVENT_GAME_NOT_LOAD);
        exit(1);
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

        // iterate over the rooms
        if (config->numRooms == 0) {
            return "No rooms available\n";
        }
    
        // create a string to store the rooms
        char *rooms = (char *)malloc(1024);
        memset(rooms, 0, 1024);

        for (int i = 0; i < config->numRooms; i++) {
            // can only join rooms that are not running
            if (config->rooms[i]->isGameRunning == false) {
                // show number of players in the room, max players in the room and the game ID
                char roomString[100];
                sprintf(roomString, "Room ID: %d, Players: %d/%d, Game ID: %d\n", config->rooms[i]->id, config->rooms[i]->numPlayers, config->rooms[i]->maxPlayers, config->rooms[i]->game->id);
                strcat(rooms, roomString);
            }
        }

        // if no rooms are available
        if (strlen(rooms) == 0) {
            return "No rooms available\n";
        }

        // return the rooms
        return rooms;
}
