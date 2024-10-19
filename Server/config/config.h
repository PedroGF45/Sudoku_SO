#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char gamePath[256];
    char logPath[256];
} ServerConfig;

ServerConfig getServerConfig(char *configPath);

#endif // CONFIG_H