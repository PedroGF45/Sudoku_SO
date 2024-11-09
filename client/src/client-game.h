#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

#include <stdbool.h>

// Estrutura para armazenar estatísticas da resolução de uma linha
typedef struct {
    double tempoResolucao;
    int tentativas;
    int acertos;
    double percentagemAcerto;
} EstatisticasLinha;

// Função para verificar a linha no buffer
int verifyLine(char *buffer);

// Função para resolver uma linha
void resolveLine(char *buffer, char *line, int row, int difficulty, EstatisticasLinha *estatisticas);

bool isValid(JSON_Array *board_array, int row, int col, int num, int difficulty);

#endif // CLIENT_GAME_H
