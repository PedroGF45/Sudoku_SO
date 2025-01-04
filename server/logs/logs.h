#ifndef LOGS_H
#define LOGS_H

#include "../config/config.h"

// Função externa para registar um erro no log e terminar o programa.
void err_dump(ServerConfig *config, int idJogo, int idJogador, char *msg, char *event);

// add message to log buffer
void addLogMessage(ServerConfig *config, char *message);

// get message from log buffer
char *getLogMessage(ServerConfig *config);

// remove message from log buffer
void removeLogMessage(ServerConfig *config);

// consume log message
void *consumeLog(void *arg);

// produce log message
void produceLog(ServerConfig *config, char *msg, char* event, int idJogo, int idJogador);

#endif // LOGS_H