#ifndef LOGS_H
#define LOGS_H

#include "../config/config.h"

// Função externa para registar um erro no log e terminar o programa.
void err_dump_client(char *filePath, int idJogo, int idJogador, char *msg, char *event);


#endif // LOGS_H