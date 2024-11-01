#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
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

void resolveLine(char *buffer, char * line, int row, int difficulty) {

    // Seed random number generator
    srand(time(NULL));

    // Parse the JSON object from the buffer
    JSON_Value *root_value = json_parse_string(buffer);
    JSON_Object *root_object = json_value_get_object(root_value);
    JSON_Array *board_array = json_object_get_array(root_object, "board");

    // Get the line array from the board array
    JSON_Array *linha_array = json_array_get_array(board_array, row);

    // Iterate through each cell in the line
    for (int i = 0; i < 9; i++) {
        int cell_value = (int)json_array_get_number(linha_array, i);

        // If the cell is empty (value is 0)
        if (cell_value == 0) {
            // Try different numbers until a valid one is found
            for (int num = 1; num <= 9; num++) {
                // Check if the number is valid for this cell
                if (isValid(board_array, row, i, num, difficulty)) {

                    // Set the cell value to the number
                    line[i] = num + '0'; // Convert to character for string representation
                }
            }
        } else {
            // The cell is already filled, copy the value to the line
            line[i] = cell_value + '0';
        }
    }

    // terminate the string
    line[9] = '\0';

    // Free the JSON objects
    json_value_free(root_value);

    printf("Linha gerada: %s\n", line); // Print the generated line
}

bool isValid(JSON_Array *board_array, int row, int col, int num, int difficulty) {

    // Check row
    for (int i = 0; i < 9; i++) {
        if (i != col && json_array_get_number(json_array_get_array(board_array, row), i) == num) {
            return false;
        }
    }

    // Check column
    if (difficulty >=2) {
        for (int i = 0; i < 9; i++) {
            if (i != row && json_array_get_number(json_array_get_array(board_array, i), col) == num) {
                return false;
            }
        }
    }
    
    // Check 3x3 subgrid
    if (difficulty == 3) {
        int startRow = (row / 3) * 3;
        int startCol = (col / 3) * 3;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (i + startRow != row && j + startCol != col &&
                    json_array_get_number(json_array_get_array(board_array, i + startRow), j + startCol) == num) {
                    return false;
                }
            }
        }
    }
    return true;
}