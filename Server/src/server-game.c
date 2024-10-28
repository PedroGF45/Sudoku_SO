#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include <string.h> // Usar strings (strcpy(), strcmp())
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"
#include "server-game.h"

// Funcao que vai carregar um dos games do ficheiro 'games.json' de acordo com o ID do game
Game loadGame(ServerConfig config, int gameID, int playerID) {
    Game game;
    FILE *file = fopen(config.gamePath, "r");

    // set default value of id as 0
    game.id = 0;

    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de game");
        exit(1);
    }

    // Ler o ficheiro JSON
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);
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
            game.id = currentID;

             // Obter o board e a solution do JSON
            for (int i = 0; i < 9; i++) {
                JSON_Array *board_row = json_array_get_array(json_object_get_array(game_object, "board"), i);
                JSON_Array *solution_row = json_array_get_array(json_object_get_array(game_object, "solution"), i);
                for (int j = 0; j < 9; j++) {
                    game.board[i][j] = (int)json_array_get_number(board_row, j);
                    game.solution[i][j] = (int)json_array_get_number(solution_row, j);
                }
            }
      

            // game has been loaded
            writeLogJSON(config.logPath, gameID, playerID, EVENT_GAME_LOAD);

            // if game has been found leave loop
            break;
        } 
    }

    // Game not found
    if (game.id == 0) {
        
        // cria mensagem mais detalhada
        char logMessage[100];
        sprintf(logMessage, "%s: game com ID %d nao encontrado", EVENT_GAME_NOT_LOAD, gameID);

        // escreve log de game nao encontrado
        writeLogJSON(config.logPath, gameID, playerID, logMessage);

        // mostra no terminal e encerra programa
        fprintf(stderr, "game com ID %d nao encontrado.\n", gameID);
        exit(1);
    }

    // Limpar memoria alocada ao JSON
    json_value_free(root_value);

    return game;
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
