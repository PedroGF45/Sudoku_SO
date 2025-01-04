#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../utils/parson/parson.h"
#include "../../utils/logs/logs-common.h"
#include "../logs/logs.h"
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
    estatisticas->percentagemAcerto = (estatisticas->acertos * 100) / (float)estatisticas->tentativas;

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

void playGame(int *socketfd, clientConfig *config) {

    // buffer for the board
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    printf("A jogar...\n");

    // set default values of reads and writes
    config->readsCount = 0;
    config->writesCount = 0;

    char *board;
    board = showBoard(socketfd, config);

    // get the current line
    char *boardSplit = strtok(board, "\n");
    char *token = strtok(NULL, "\n");
    int currentLine = atoi(token);

    char tempString[BUFFER_SIZE]; // Allocate a temporary buffer
    strcpy(tempString, boardSplit); // Copy the original board data

    printf("Linha atual: %d\n", currentLine);

    EstatisticasLinha *estatisticas;
    estatisticas = (EstatisticasLinha *)malloc(sizeof(EstatisticasLinha));
    estatisticas->tentativas = 0; // Iniciar com 0 tentativas
    estatisticas->acertos = 0;
    estatisticas->percentagemAcerto = 0.0;
    estatisticas->tempoResolucao = 0.0;

    free(board);
    writeLogJSON(config->logPath, 0, config->clientID, "Started playing the game");

    // Enviar linhas inseridas pelo utilizador e receber o board atualizado
    while (currentLine <= 9) {

        int validLine = 0;  // Variável para controlar se a linha está correta

        // line to send to server
        char line[10];

        // inicializa o buffer com '0' e terminador nulo
        memset(line, '0', sizeof(line));

        while (!validLine) {

            if (config->isManual) {

                printf("Insira valores para a linha %d do board (exactamente 9 digitos):\n", currentLine);
                scanf("%s", line);
                char logMessage[256];
                snprintf(logMessage, sizeof(logMessage), "Manual input for board line %d", currentLine);
                writeLogJSON(config->logPath, 0, config->clientID, logMessage);

            } else {

                // Passa a variável estatisticas para a função resolveLine
                resolveLine(tempString, line, currentLine - 1, config->difficulty, estatisticas);
                char logMessage[256];
                snprintf(logMessage, sizeof(logMessage), "Auto-solving the board line %d", currentLine);
                writeLogJSON(config->logPath, 0, config->clientID, logMessage);

            }

            // Enviar a linha ao servidor
            if (send(*socketfd, line, sizeof(line), 0) < 0) {
                err_dump_client(config->logPath, 0, config->clientID, "can't send lines to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
                continue;
            } else {
                printf("Linha enviada: %s\n", line);
                // Incrementa o contador de escritas
                config->writesCount++;
                char logMessage[256];
                snprintf(logMessage, sizeof(logMessage), "Sent line %d to server", currentLine);
                writeLogJSON(config->logPath, 0, config->clientID, logMessage);
            }

            char *board;
            board = showBoard(socketfd, config);
            // get the current line
            boardSplit = strtok(board, "\n");
            char *token = strtok(NULL, "\n");
            int serverLine = atoi(token);

            strcpy(tempString, boardSplit); // Copy the original board data

            //printf("Linha do servidor: %d\n", serverLine);
            if (serverLine > currentLine) {
                validLine = 1;
                currentLine = serverLine;
                char logMessage[256];
                snprintf(logMessage, sizeof(logMessage), "Received line %d from server", currentLine);
                writeLogJSON(config->logPath, 0, config->clientID, logMessage);
            } else {
                printf("Linha %d incorreta. Tente novamente.\n", currentLine);
                char logMessage[256];
                snprintf(logMessage, sizeof(logMessage), "Received incorrect line %d from server", currentLine);
                writeLogJSON(config->logPath, 1, config->clientID, logMessage);
            }

            // print read and write counts
            printf("Number of Reads: %d\n", config->readsCount);
            printf("Number of Writes: %d\n", config->writesCount);

            free(board);
        }
    }

    finishGame(socketfd, config, estatisticas);
    writeLogJSON(config->logPath, 0, config->clientID, "Game finished");
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

char *showBoard(int *socketfd, clientConfig *config) {

    // buffer for the board
    // Allocate memory for buffer on the heap
    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }
    memset(buffer, 0, BUFFER_SIZE);

    printf("Received board from server...\n");

    // receive the board from the server
    if (recv(*socketfd, buffer, BUFFER_SIZE, 0) < 0) {
        // error receiving board from server
        err_dump_client(config->logPath, 0, config->clientID, "can't receive board from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);
        free(buffer);

    } else {

        if (strcmp(buffer, "No rooms available") == 0) {
            printf("No rooms available\n");
            free(buffer);
            return NULL;
        }
    }

    //printf("Board received: %s\n", buffer);

    char *board = strtok(buffer, "\n");
    char *token = strtok(NULL, "\n");
    int serverLine = atoi(token);
    //printf("Linha do servidor: %d\n", serverLine);

    // get the JSON object from the buffer
    JSON_Value *root_value = json_parse_string(board);
    JSON_Object *root_object = json_value_get_object(root_value);

    // get the game ID
    int gameID = (int)json_object_get_number(root_object, "id");

    // print the board
    printf("-------------------------------------\n");
    printf("BOARD ID: %d  PLAYER ID: %d   %s\n", gameID, config->clientID, config->isPremium ? "PREMIUM" : "NON-PREMIUM");
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

    // Concatenate board and server line
    char tempString[BUFFER_SIZE]; // Allocate a temporary buffer
    strcpy(tempString, board); // Copy the original board data
    strcat(tempString, "\n"); // Concatenate newline
    char serverLineStr[10];
    sprintf(serverLineStr, "%d", serverLine);
    strcat(tempString, serverLineStr); // Concatenate server line

    // Copy the modified string to the original buffer
    strcpy(buffer, tempString);

    // Free the JSON object
    json_value_free(root_value);

    writeLogJSON(config->logPath, gameID, config->clientID, EVENT_BOARD_SHOW);

    // increase the reads count
    config->readsCount++;

    return buffer;
}

void finishGame(int *socketfd, clientConfig *config, EstatisticasLinha *estatisticas) {
    
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // send the accuracy to the server
    char accuracyString[10];
    sprintf(accuracyString, "%.2f", estatisticas->percentagemAcerto);

    // send the accuracy to the server
    if (send(*socketfd, accuracyString, strlen(accuracyString), 0) < 0) {

        // error sending accuracy to server
        err_dump_client(config->logPath, 0, config->clientID, "can't send accuracy to server", EVENT_MESSAGE_CLIENT_NOT_SENT);
    } else {

        // show the accuracy sent
        //printf("Accuracy sent: %s\n", accuracyString);
         // Log accuracy sent
        char logMessage[256];
        snprintf(logMessage, sizeof(logMessage), "%s: sent accuracy: %s", EVENT_MESSAGE_CLIENT_SENT, accuracyString);
        writeLogJSON(config->logPath, 0, config->clientID, logMessage); // Log de envio de precisão
    }

    printf("Tentativas: %d\n", estatisticas->tentativas);
    printf("Acertos: %d\n", estatisticas->acertos);
    printf("Percentagem de acerto: %.2f%%\n", estatisticas->percentagemAcerto);

    

    // receive the final message from the server
    if (recv(*socketfd, buffer, sizeof(buffer), 0) < 0) {

        // error receiving final board from server
        err_dump_client(config->logPath, 0, config->clientID, "can't receive final board from server", EVENT_MESSAGE_CLIENT_NOT_RECEIVED);

    } else {

        // show the final message
        printf("%s", buffer);
    }

    free(estatisticas);
}