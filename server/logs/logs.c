#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "logs.h"
#include "../../utils/logs/logs-common.h"


void err_dump(ServerConfig * config, int idJogo, int idJogador, char *msg, char *event) {
	
	// produce log message
    produceLog(config, msg, event, idJogo, idJogador);

	// imprime a mensagem de erro e termina o programa
	perror(msg);
	exit(1);
}

// add message to log buffer
void addLogMessage(ServerConfig *config, char *message) {

    // add message to log buffer
    for (int i = 0; i < 10; i++) {
        if (config->logBuffer[i][0] == '\0') {
            strcpy(config->logBuffer[i], message);
            break;
        }
    }
}

// get message from log buffer
char *getLogMessage(ServerConfig *config) {
    
    // get message from log buffer
    for (int i = 0; i < 10; i++) {
        if (config->logBuffer[i][0] != '\0') {
            return config->logBuffer[i];
        }
    }

    return NULL;
}

// remove message from log buffer
void removeLogMessage(ServerConfig *config) {
    
    // remove message from log buffer
    for (int i = 0; i < 10; i++) {
        if (config->logBuffer[i][0] != '\0') {
            memset(config->logBuffer[i], 0, sizeof(config->logBuffer[i]));
            break;
        }
    }
}


void *consumeLog(void *arg) {

    while (1) {

        ServerConfig* config = (ServerConfig*) arg; // get config

        sem_wait(&config->itemsLogSemaphore);   // check if there are items to consume

        printf("SERVER detects a log to write!\n");

        sem_wait(&config->mutexLogSemaphore);   // lock the buffer

        printf("SERVER writing a log message\n");

        char *message = getLogMessage(config);  // get message from buffer
        if (message != NULL) {                  

            // break message into gameID, playerID and message through \n
            char *token = strtok(message, "\n");
            int gameID = atoi(token);
            token = strtok(NULL, "\n");
            int playerID = atoi(token);
            token = strtok(NULL, "\n");
            char *message = token;

            writeLogJSON(config->logPath, gameID, playerID, message);  // write the message on the log
            removeLogMessage(config);           // remove message from buffer
        }
        sem_post(&config->mutexLogSemaphore);   // unlock the buffer

        printf("SERVER finished writing a log message\n");
        sem_post(&config->spacesSemaphore);     // signal that there are spaces to produce
        printf("SERVER signaled that there are spaces to produce\n");
    }

    return NULL;
}

void produceLog(ServerConfig *config, char *msg, char* event, int idJogo, int idJogador) {

    printf("CLIENT %d : wants to write a log message\n", idJogador);

    sem_wait(&config->spacesSemaphore);     // check if there is space to produce

    printf("THERE's space to CLIENT %d write a log message!\n", idJogador);
    sem_wait(&config->mutexLogSemaphore);   // lock the buffer

    printf("CLIENT %d : writing a log message\n", idJogador);
    char *message = concatenateInfo(msg, event, idJogo, idJogador);  // concatenate info
    addLogMessage(config, message);         // add message to buffer

    sem_post(&config->mutexLogSemaphore);   // unlock the buffer
    sem_post(&config->itemsLogSemaphore);   // signal that there are items to consume
}