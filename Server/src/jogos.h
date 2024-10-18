#ifndef JOGOS_H
#define JOGOS_H

#include "C:\Users\claud\OneDrive\Documentos\GitHub\Sudoku_SO\parson.h"
/*Struct para armazenar o tabuleiro e a solucao do jogo.
Aqui usei o typedef para criar um alias para a struct 
(evita repetir 'struct' no código sempre que quisermos usar a struct). */

typedef struct {
    char tabuleiro[9][9];  // Tabuleiro (9x9)
    char solucao[9][9];    // Solucao correta
} Jogo;

// Carrega o jogo do ficheiro
Jogo carregaJogo(const char *filename, int idJogo);

// Verifica a solucao do jogo
int verificaSolucao(Jogo jogo, int solucaoEnviada[9][9]);

void mostraTabuleiro(Jogo jogo);

// nova funcao para verificar linha a linha
int verificaLinha(Jogo *jogo, int linhaInserida[9], int numeroLinha);

#endif