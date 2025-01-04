#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "logs.h"
#include "../../utils/logs/logs-common.h"


void err_dump_client(char *filePath, int idJogo, int idJogador, char *msg, char *event) {
	
	// produce log message
    writeLogJSON(filePath, idJogo, idJogador, msg);

	// imprime a mensagem de erro e termina o programa
	perror(msg);
	exit(1);
}
