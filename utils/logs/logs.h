#ifndef LOGS_H
#define LOGS_H

#define EVENT_SERVER_START "Server inicializado"
#define EVENT_GAME_LOAD "Jogo carregado"
#define EVENT_GAME_NOT_LOAD "Jogo nao carregado"
#define EVENT_BOARD_SHOW "Tabuleiro do jogo mostrado"
#define EVENT_SOLUTION_SENT "Solucao recebida"
#define EVENT_SOLUTION_CORRECT "Solucao correta"
#define EVENT_SOLUTION_WRONG "Solucao errada"

struct log {
    int id;
    char event[32];
    char log[256];
};

// wirte log in JSON format
void writeLogJSON(const char *filename, int idJogo, int idJogador, const char *logMessage);

#endif