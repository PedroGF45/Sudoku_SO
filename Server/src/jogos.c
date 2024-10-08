#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include <string.h> // Usar strings (strcpy(), strcmp())
#include "jogos.h"  // Incluir o header com a struct e funcoes

// Funcao que vai carregar um dos jogos do ficheiro 'games.txt'
Jogo carregaJogo(const char *filename, int idJogo) {
    Jogo jogo;
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de jogo");
        exit(1);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), file)) {
        int id;
        char tabuleiro[82], solucao[82];

        // Ler a linha e extrair o ID, tabuleiro e solucao
        sscanf(linha, "%d,%81s,%81s", &id, tabuleiro, solucao);

        // Se o ID do jogo corresponder, armazenar o tabuleiro e a solucao
        if (id == idJogo) {
            strcpy(jogo.tabuleiro, tabuleiro);
            strcpy(jogo.solucao, solucao);
            break;
        }
    }

    fclose(file);
    return jogo;
}

// Funcao para verificar se a solucao enviada esta correta
int verificaSolucao(Jogo jogo, const char *solucaoEnviada) {
    // Comparar a solucao enviada com a solucao correta
    return strcmp(jogo.solucao, solucaoEnviada) == 0;
}