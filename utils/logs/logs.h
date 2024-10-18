#ifndef LOGS_H
#define LOGS_H

#define EVENT_GAME_LOAD "Jogo carregado"
#define EVENT_SOLUTION_SENT "Solucao enviada"
#define EVENT_SOLUTION_CORRECT "Solucao correta"
#define EVENT_SOLUTION_WRONG "Solucao errada"
#define EVENT_BOARD_SHOW "Tabuleiro do jogo mostrado"

struct log {
    int id;
    char event[32];
    char log[256];
};

void writeLog(const char *filename, int idJogo, int idJogador, const char *log);

// novo log
void writeLogJSON(const char *filename, int idJogo, int idJogador, const char *logMessage);

#endif