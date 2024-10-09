#ifndef JOGOS_H
#define JOGOS_H

/*Struct para armazenar o tabuleiro e a solucao do jogo.
Aqui usei o typedef para criar um alias para a struct 
(evita repetir 'struct' no código sempre que quisermos usar a struct). */

typedef struct {
    char tabuleiro[82];  // Tabuleiro (9x9)
    char solucao[82];    // Solucao correta
} Jogo;

// Carrega o jogo do ficheiro
Jogo carregaJogo(const char *filename, int idJogo);

// Verifica a solucao do jogo
int verificaSolucao(Jogo jogo, const char *solucaoEnviada);

void mostraTabuleiro(Jogo jogo);

#endif