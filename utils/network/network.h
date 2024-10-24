#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 1024

extern int fd;
extern char *ptr;
extern int nbytes;
extern int readn(int fd, char *ptr, int nbytes);
extern int writen(int fd, char *ptr, int nbytes);
extern int readline(int fd, char *ptr, int maxlen);
extern void err_dump(char *logPath, int idJogo, int idJogador, char *msg, char *event);
extern void str_cli(FILE *fp, int sockfd);
extern void str_echo(int sockfd);