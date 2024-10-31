#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"
#include "client-game.h"

int verifyLine(char *buffer) {
    if (strlen(buffer) != 9) {
        return -1;
    }
    for (int i = 0; i < 9; i++) {
        if (buffer[i] < '0' || buffer[i] > '9') {
            return -1;
        }
    }
    return 0;
}

void showBoard(char *buffer, char * logFileName, int playerID) {

    // get the JSON object from the buffer
    JSON_Value *root_value = json_parse_string(buffer);
    JSON_Object *root_object = json_value_get_object(root_value);

    // get the game ID
    int gameID = (int)json_object_get_number(root_object, "id");

    // print the board
    printf("-------------------------------------\n");
    printf("BOARD ID: %d       PLAYER ID: %d\n", gameID, playerID);
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

    writeLogJSON(logFileName, gameID, playerID, EVENT_BOARD_SHOW);
}

void getRandomLine(char *buffer) {

    // Seed random number generator
    srand(time(NULL));

    // Generate random line
    for (int i = 0; i < 9; i++) {
        buffer[i] = (rand() % 9) + '0'; // Random number between 0 and 9 ASCII
    }

    buffer[9] = '\0';

    printf("Linha gerada: %s\n", buffer);
}
