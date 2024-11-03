#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"
#include "client-game.h"


/**
 * Verifica se a linha contém exatamente 9 dígitos numéricos (de '0' a '9').
 *
 * @param buffer Uma string que contém a linha a ser verificada.
 * @return -1 se a linha não tiver exatamente 9 caracteres ou se contiver caracteres não numéricos.
 *         0 se a linha for válida (contém exatamente 9 dígitos numéricos).
 *
 * @details A função faz o seguinte:
 * - Verifica se o comprimento da string `buffer` é exatamente 9.
 * - Percorre cada carácter na string para verificar se é um dígito numérico.
 * - Se qualquer carácter não for um dígito ou se o comprimento for diferente de 9,
 * retorna -1; caso contrário, retorna 0.
 */

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


/**
 * Exibe o tabuleiro de jogo a partir de uma string JSON e regista o evento no log.
 *
 * @param buffer Uma string JSON que contém o estado do tabuleiro.
 * @param logFileName O caminho para o ficheiro de log onde o evento será registado.
 * @param playerID O identificador do jogador que está a visualizar o tabuleiro.
 *
 * @details A função faz o seguinte:
 * - Faz o parse da string JSON para obter o objeto `board` e o `gameID`.
 * - Imprime o tabuleiro no formato de uma grelha 9x9 com separadores visuais.
 * - Regista o evento de visualização do tabuleiro no ficheiro de log.
 * - Liberta a memória alocada para o objeto JSON após a operação.
 */

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


/**
 * Resolve uma linha do tabuleiro de jogo, preenchendo células vazias com números válidos.
 *
 * @param buffer Uma string JSON que contém o estado atual do tabuleiro.
 * @param line Uma string de 10 caracteres onde a linha resolvida será armazenada (9 dígitos + terminador nulo).
 * @param row O número da linha (0-indexado) que será resolvida.
 * @param difficulty O nível de dificuldade usado para validar os números inseridos.
 *
 * @details A função faz o seguinte:
 * - Inicializa o gerador de números aleatórios.
 * - Faz o parse da string JSON para obter o tabuleiro de jogo.
 * - Itera por cada célula da linha especificada:
 *   - Se a célula estiver vazia (valor 0), tenta números de 1 a 9 até encontrar um válido.
 *   - Se a célula já tiver um valor, copia-o para a string `line`.
 * - Usa a função `isValid` para verificar se um número é válido para a posição dada, tendo em conta a dificuldade.
 * - Termina a string `line` com o caractere nulo (`'\0'`) e liberta a memória alocada para o objeto JSON.
 * - Imprime a linha gerada no terminal.
 */

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


/**
 * Verifica se um número pode ser colocado numa célula específica do tabuleiro de acordo 
 * com as regras do jogo e o nível de dificuldade.
 *
 * @param board_array Um pointer para o array JSON que representa o tabuleiro de jogo.
 * @param row O índice da linha da célula a ser verificada.
 * @param col O índice da coluna da célula a ser verificada.
 * @param num O número a ser verificado.
 * @param difficulty O nível de dificuldade que determina as regras de verificação (1, 2 ou 3).
 * @return `true` se o número puder ser colocado na célula sem violar as regras, `false` caso contrário.
 *
 * @details A função verifica a validade do número nas seguintes condições:
 * - **Linha**: O número não pode já existir na mesma linha (exceto na coluna atual).
 * - **Coluna**: Se a dificuldade for 2 ou superior, o número não pode já existir na mesma coluna (exceto na linha atual).
 * - **Subgrade 3x3**: Se a dificuldade for 3, o número não pode já existir na mesma subgrade 3x3 (exceto na célula atual).
 * 
 * @note A função ajusta a complexidade da verificação com base no nível de dificuldade fornecido:
 * - Dificuldade 1: Apenas verifica a linha.
 * - Dificuldade 2: Verifica a linha e a coluna.
 * - Dificuldade 3: Verifica a linha, a coluna, e a subgrade 3x3.
 */

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