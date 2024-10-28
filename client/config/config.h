#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char serverIP[256];
    int serverPort;
    char serverHostName[256];
    int clientID;
    char logPath[256];
} clientConfig;

clientConfig getClientConfig(char *configPath);

#endif // CONFIG_H