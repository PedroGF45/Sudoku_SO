#ifndef LOGS_H
#define LOGS_H

#define EVENT_SERVER 1
#define EVENT_CLIENT 2

struct log {
    int id;
    char event[32];
    char log[256];
};

void writeLog(const char *filename, const char *log);

#endif