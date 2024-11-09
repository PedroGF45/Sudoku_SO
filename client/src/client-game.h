#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H


// Verifica se a linha no buffer é válida (exatamente 9 dígitos numéricos).
int verifyLine(char *buffer);

// Resolve uma linha do tabuleiro, preenchendo células vazias com números válidos.
void resolveLine(char *buffer, char *line, int row, int difficulty);

// Verifica se um número pode ser colocado numa célula específica do tabuleiro.
bool isValid(JSON_Array *board_array, int row, int col, int num, int difficulty);

#endif