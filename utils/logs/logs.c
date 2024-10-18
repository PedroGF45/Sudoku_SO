#include <stdio.h>  // Usar FILE, fopen(), fclose()
#include <stdlib.h> 
#include "logs.h"  // Incluir o header com a struct e funcoes
#include <time.h>  // Usar time_t, time(), ctime()
#include "C:\\Users\\claud\\OneDrive\\Documentos\\GitHub\\Sudoku_SO\\parson.h"

void writeLog(const char *filename, int idJogo, int idJogador, const char *log) {

    // Abrir o ficheiro em modo de escrita no final do ficheiro
    FILE *file = fopen(filename, "a");

    // Verificar se o ficheiro foi aberto corretamente
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de log");
        exit(1);
    }

    int idLog = 0;
    // Obter a data e hora atual
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // Escrever o log no ficheiro
    fprintf(file, "%02d/%02d/%04d %02d:%02d:%02d %d %d %d %s\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, idLog, idJogo, idJogador, log);

    // Fechar o ficheiro
    fclose(file);
}


// novo log JSON
void writeLogJSON(const char *filename, int idJogo, int idJogador, const char *logMessage) {
    // Abrir o ficheiro JSON existente
    JSON_Value *rootValue = json_parse_file(filename);
    JSON_Object *rootObject = NULL;
    JSON_Value *logsArrayValue = NULL;
    JSON_Array *logsArray = NULL;

    if (rootValue == NULL) {
        // Se o ficheiro nao existe ou esta vazio, cria uma nova estrutura JSON
        rootValue = json_value_init_object();
        rootObject = json_value_get_object(rootValue);
        logsArrayValue = json_value_init_array();  // Criar um array vazio para os logs
        json_object_set_value(rootObject, "logs", logsArrayValue);  // Adicionar o array ao objeto raiz
    } else {
        // Se o ficheiro JSON ja existe, carregar os dados existentes
        rootObject = json_value_get_object(rootValue);
        logsArrayValue = json_object_get_value(rootObject, "logs");
    }

    logsArray = json_value_get_array(logsArrayValue);  // Obter o array de logs

    // Obter a data e hora atual
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char timestamp[20];
    // Aqui alterei o formato da data para nao ser necessario dar escape da / no ficheiro JSON
    sprintf(timestamp, "%02d-%02d-%04d %02d:%02d:%02d", 
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Cria um novo ficheiro JSON para o log mesmo que nao tenha sido criado antes
    JSON_Value *logValue = json_value_init_object();
    JSON_Object *logObject = json_value_get_object(logValue);

    // Preencher os campos do log
    json_object_set_string(logObject, "timestamp", timestamp);
    json_object_set_number(logObject, "idJogo", idJogo);
    json_object_set_number(logObject, "idJogador", idJogador);
    json_object_set_string(logObject, "mensagem", logMessage);

    // Adicionar o novo log ao array de logs
    json_array_append_value(logsArray, logValue);

    // Gravar o ficheiro JSON atualizado
    if (json_serialize_to_file_pretty(rootValue, filename) != JSONSuccess) {
        printf("Erro ao gravar o ficheiro JSON: %s\n", filename);
    }

    // Limpar a memória JSON
    json_value_free(rootValue);
}
