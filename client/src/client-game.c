#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../utils/logs/logs-common.h"
#include "../logs/logs.h"
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

void resolveLine(char *buffer, char *line, int row, int difficulty, EstatisticasLinha *estatisticas) {
    // Inicializar as estatísticas
    estatisticas->tentativas = 0; // Iniciar com 0 tentativas
    estatisticas->acertos = 0;
    estatisticas->percentagemAcerto = 0.0;
    estatisticas->tempoResolucao = 0.0;
    // Seed random number generator
    srand(time(NULL));

    printf("Resolvendo linha %d...\n", row + 1);
    //printf("Buffer recebido: %s\n", buffer);

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
                estatisticas->tentativas++; // Aumenta o número de tentativas a cada tentativa de número

                // Check if the number is valid for this cell
                if (isValid(board_array, row, i, num, difficulty)) {
                    // Set the cell value to the number
                    line[i] = num + '0'; // Convert to character for string representation

                    // Se o número for o correto, aumenta o número de acertos
                    if (num == (int)json_array_get_number(linha_array, i)) {
                        estatisticas->acertos++;
                    }
                    break; // Se for encontrado um número válido para de aumentar os acertos
                }
            }
        } else {
            // The cell is already filled, copy the value to the line
            line[i] = cell_value + '0';
            estatisticas->acertos++; // Aumenta o número de acertos
        }
    }

    // Terminate the string
    line[9] = '\0';

    // Free the JSON objects
    json_value_free(root_value);

    // Calcular a percentagem de acerto
    estatisticas->percentagemAcerto = (float)estatisticas->acertos / 9 * 100;

    // Exibir as estatísticas 
    //printf("Linha gerada: %s\n", line);
    printf("Tentativas: %d\n", estatisticas->tentativas);
    printf("Acertos: %d\n", estatisticas->acertos);
    printf("Percentagem de acerto: %.2f%%\n", estatisticas->percentagemAcerto);
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

int showTimerUpdate(char *buffer, int timeLeft) {

    //printf("Recebido: %s\n", buffer);
    // Parse the received message
    strtok(buffer, "\n");
    timeLeft = atoi(strtok(NULL, "\n"));
    //printf("Tempo restante: %d segundos\n", timeLeft);
    int roomId = atoi(strtok(NULL, "\n"));
    //printf("ID da sala: %d\n", roomId);
    int gameId = atoi(strtok(NULL, "\n"));
    //printf("ID do jogo: %d\n", gameId);
    int numPlayers = atoi(strtok(NULL, "\n"));
    //printf("Jogadores na sala: %d\n", numPlayers);

    // show the timer update
    printf("Time left: %d seconds - Room ID: %d - Game ID: %d - Players joined: %d\n", timeLeft, roomId, gameId, numPlayers);

    return --timeLeft;
}
