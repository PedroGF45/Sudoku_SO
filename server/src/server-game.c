#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include <string.h> // Usar strings (strcpy(), strcmp())
#include <time.h>   // Usar time()
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"
#include "server-game.h"

static int nextRoomID = 1;

int generateUniqueId() {
    return nextRoomID++;
}

// Funcao que vai carregar um dos games do ficheiro 'games.json' de acordo com o ID do game
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
            break;
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
        game = NULL;
    }

    // Limpar memoria alocada ao JSON
    json_value_free(root_value);

    return game;
}

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

int verifyLine(char * logFileName, char * solutionSent, Game *game, int insertLine[9], int lineNumber, int playerID) {
    int correct = 1;
    char logMessage[100];
    
    sprintf(logMessage, "%s para a linha %d: %s", EVENT_SOLUTION_SENT, lineNumber + 1, solutionSent);
    writeLogJSON(logFileName, game->id, playerID, logMessage);
    
    for (int j = 0; j < 9; j++) {
        if (game->board[lineNumber][j] != 0) {
            continue;  // Ignora valores fixos
        }

        if (insertLine[j] == game->solution[lineNumber][j]) {
            game->board[lineNumber][j] = insertLine[j];
        } else {
            correct = 0;
            printf("Erro na posição %d: esperado %d, recebido %d\n", j + 1, game->solution[lineNumber][j], insertLine[j]);
        }
    }

    if (correct) {
        writeLogJSON(logFileName, game->id, playerID, "Linha validada como correct");
    } else {
        writeLogJSON(logFileName, game->id, playerID, "Linha invalidada, tentar novamente");
    }

    return correct;
}

// Criar room
Room *createRoom(ServerConfig *config) {

    Room *room = (Room *)malloc(sizeof(Room));
    if (room == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    memset(room, 0, sizeof(Room));  // Initialize Room struct

    room->id = generateUniqueId();
    room->players = (int *)malloc(config->maxPlayersPerRoom * sizeof(int));
    memset(config->rooms, 0, sizeof(Room) * config->maxRooms); // Ensures all fields in rooms are set to 0


    // increase the number of rooms
    config->numRooms++;
    
    writeLogJSON(config->logPath, 0, 0, EVENT_ROOM_LOAD);
    return room;
}

// Obter jogos existentes
char *getExistingGames(ServerConfig *config) {

    // iterar sobre as salas e obter os jogos existentes
    char *games = (char *)malloc(256);
    memset(games, 0, 256);

    for (int i = 0; i < config->maxRooms; i++) {

        // check if theres an initialized room in the server
        if (config->rooms[i] != NULL) {          

            // check if theres a game in the room
            if (config->rooms[i]->game != NULL) {

                // get the game id
                int gameID = config->rooms[i]->game->id;

                // create a menu string
                char roomID[10];
                sprintf(roomID, "%d", gameID);

                // check if its the last room
                if (i == config->maxRooms - 1) {
                    strcat(games, roomID);
                } else {
                    strcat(games, roomID);
                    strcat(games, ",");
                }
            }
        }
    }
    return games;
}
