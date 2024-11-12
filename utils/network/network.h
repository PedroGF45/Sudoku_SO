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

// Declarações de variáveis externas usadas para operações de I/O.
extern int fd;
extern char *ptr;
extern int nbytes;

// Função externa para ler nbytes de um descritor de ficheiro.
extern int readn(int fd, char *ptr, int nbytes);

// Função externa para escrever nbytes num descritor de ficheiro.
extern int writen(int fd, char *ptr, int nbytes);

// Função externa para ler uma linha de um descritor de ficheiro.
extern int readline(int fd, char *ptr, int maxlen);

// Função externa para gerir a comunicação cliente-servidor.
extern void str_cli(FILE *fp, int sockfd);

// Função externa para ecoar dados recebidos de um cliente.
extern void str_echo(int sockfd);