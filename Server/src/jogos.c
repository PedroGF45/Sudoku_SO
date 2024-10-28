#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include <string.h> // Usar strings (strcpy(), strcmp())
#include "jogos.h"  // Incluir o header com a struct e funcoes
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"

// Funcao que vai carregar um dos jogos do ficheiro 'games.json' de acordo com o ID do jogo
Jogo carregaJogo(ServerConfig config, int idJogo, int idJogador) {
    Jogo jogo;
    FILE *file = fopen(config.gamePath, "r");

    // set default value of id as 0
    jogo.id = 0;

    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de jogo");
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
        
        // se o jogo for encontrado
        if (currentID == idJogo) {

            // set game id as currentID
            jogo.id = currentID;

             // Obter o tabuleiro e a solucao do JSON
            for (int i = 0; i < 9; i++) {
                JSON_Array *tabuleiro_row = json_array_get_array(json_object_get_array(game_object, "tabuleiro"), i);
                JSON_Array *solucao_row = json_array_get_array(json_object_get_array(game_object, "solucao"), i);
                for (int j = 0; j < 9; j++) {
                    jogo.tabuleiro[i][j] = (int)json_array_get_number(tabuleiro_row, j);
                    jogo.solucao[i][j] = (int)json_array_get_number(solucao_row, j);
                }
            }
      

            // game has been loaded
            writeLogJSON(config.logPath, idJogo, idJogador, EVENT_GAME_LOAD);

            // if game has been found leave loop
            break;
        } 
    }

    // Game not found
    if (jogo.id == 0) {
        
        // cria mensagem mais detalhada
        char logMessage[100];
        sprintf(logMessage, "%s: Jogo com ID %d nao encontrado", EVENT_GAME_NOT_LOAD, idJogo );

        // escreve log de jogo nao encontrado
        writeLogJSON(config.logPath, idJogo, idJogador, logMessage);

        // mostra no terminal e encerra programa
        fprintf(stderr, "Jogo com ID %d nao encontrado.\n", idJogo);
        exit(1);
    }

    // Limpar memoria alocada ao JSON
    json_value_free(root_value);

    return jogo;
}

void mostraTabuleiro(char * logFileName, Jogo jogo, int idJogador) {

    printf("ID do jogo: %d\n", jogo.id);
    printf("-------------------------------------\n");
    for (int i = 0; i < 9; i++) {
        printf("| line %d -> | ", i + 1);
        for (int j = 0; j < 9; j++) {
            if (jogo.tabuleiro[i][j] == 0) {
                printf("0 ");  // Mostra '0' para espaços vazios
            } else {
                printf("%d ", jogo.tabuleiro[i][j]);
            }
            if ((j + 1) % 3 == 0) {
                printf("| ");
            }
        }
        if ((i + 1) % 3 == 0) {
            printf("\n-------------------------------------");
        }
        printf("\n");
    }

    writeLogJSON(logFileName, jogo.id, idJogador, EVENT_BOARD_SHOW);
}

int verificaLinha(char * logFileName, char * solucaoEnviada, Jogo *jogo, int linhaInserida[9], int numeroLinha, int idJogador) {
    int correta = 1;
    char logMessage[100];
    
    sprintf(logMessage, "%s para a linha %d: %s", EVENT_SOLUTION_SENT, numeroLinha + 1, solucaoEnviada);
    writeLogJSON(logFileName, jogo->id, idJogador, logMessage);
    
    for (int j = 0; j < 9; j++) {
        if (jogo->tabuleiro[numeroLinha][j] != 0) {
            continue;  // Ignora valores fixos
        }

        if (linhaInserida[j] == jogo->solucao[numeroLinha][j]) {
            jogo->tabuleiro[numeroLinha][j] = linhaInserida[j];
        } else {
            correta = 0;
            printf("Erro na posição %d: esperado %d, recebido %d\n", j + 1, jogo->solucao[numeroLinha][j], linhaInserida[j]);
        }
    }

    if (correta) {
        writeLogJSON(logFileName, jogo->id, idJogador, "Linha validada como correta");
    } else {
        writeLogJSON(logFileName, jogo->id, idJogador, "Linha invalidada, tentar novamente");
    }

    return correta;
}

int validarLinha(char *buffer) {
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

