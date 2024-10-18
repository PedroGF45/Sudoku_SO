#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include <string.h> // Usar strings (strcpy(), strcmp())
#include "jogos.h"  // Incluir o header com a struct e funcoes
#include "C:\Users\claud\OneDrive\Documentos\GitHub\Sudoku_SO\parson.h"

// Funcao que vai carregar um dos jogos do ficheiro 'games.txt'
Jogo carregaJogo(const char *filename, int idJogo) {
    Jogo jogo;
    FILE *file = fopen(filename, "r");

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

    // Obter o tabueliro e a solucao do JSON
    for (int i = 0; i < 9; i++) {
        JSON_Array *tabuleiro_row = json_array_get_array(json_object_get_array(root_object, "tabuleiro"), i);
        JSON_Array *solucao_row = json_array_get_array(json_object_get_array(root_object, "solucao"), i);
        for (int j = 0; j < 9; j++) {
            jogo.tabuleiro[i][j] = (int)json_array_get_number(tabuleiro_row, j);
            jogo.solucao[i][j] = (int)json_array_get_number(solucao_row, j);
        }
    }

    // Limpar memoria alocada ao JSON
    json_value_free(root_value);

    return jogo;
}
// Nao vai ser necessario esta funcao se quisermos manter esta logica de linha a linha
// Funcao para verificar se a solucao enviada esta correta
int verificaSolucao(Jogo jogo, int solucaoEnviada[9][9]) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (jogo.solucao[i][j] != solucaoEnviada[i][j]) {
                return 0;  // Solução errada
            }
        }
    }
    return 1;  // Solução correta
}

void mostraTabuleiro(Jogo jogo) {
    printf("-------------------------\n");
    for (int i = 0; i < 9; i++) {
        printf("| ");
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
            printf("\n-------------------------");
        }
        printf("\n");
    }
}

// Funcao para verificar uma linha e corrigir valores incorretos
int verificaLinha(Jogo *jogo, int linhaInserida[9], int numeroLinha) {
    int correta = 1;  // Assume que a linha está correta
    for (int j = 0; j < 9; j++) {
        // Verifica se a posicao no tabuleiro original ja tem um valor fixo
        if (jogo->tabuleiro[numeroLinha][j] != 0) {
            continue;  // Nao vai alterar os valores que ja lae stao
        }
        
        // Se a posicao esta vazia no tabuleiro original, faz a verificacao
        if (linhaInserida[j] == jogo->solucao[numeroLinha][j]) {
            // O valor esta correto, inserir no tabuleiro
            jogo->tabuleiro[numeroLinha][j] = linhaInserida[j];
        } else {
            // O valor esta errado, substituí-lo por 0 no tabuleiro
            jogo->tabuleiro[numeroLinha][j] = 0;
            correta = 0;  // A linha nao esta correta
        }
    }
    return correta;  // Retorna 1 se toda a linha estiver correta, 0 se não
}