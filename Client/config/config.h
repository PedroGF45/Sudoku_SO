#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char serverIP[256];
    char serverPort[256];
    char clientID[256];
    char logPath[256];
} clientConfig;

clientConfig getClientConfig(char *configPath);

#endif // CONFIG_H