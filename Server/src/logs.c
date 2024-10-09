#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include "logs.h"  // Incluir o header com a struct e funcoes
#include <time.h>  // Usar time_t, time(), ctime()

void writeLog(const char *filename, const char *log, int id) {

    // Abrir o ficheiro em modo de escrita no final do ficheiro
    FILE *file = fopen(filename, "a");

    // Verificar se o ficheiro foi aberto corretamente
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de log");
        exit(1);
    }

    // Obter a data e hora atual
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // Escrever o log no ficheiro
    fprintf(file, "%d | %02d:%02d:%02d | %s\n", id, tm.tm_hour, tm.tm_min, tm.tm_sec, log);

    // Fechar o ficheiro
    fclose(file);
}
