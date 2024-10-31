#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    char serverIP[256];
    int serverPort;
    char serverHostName[256];
    int clientID;
    char logPath[256];
    bool isManual;
} clientConfig;

int generateClientID();

clientConfig getClientConfig(char *configPath);

#endif // CONFIG_H