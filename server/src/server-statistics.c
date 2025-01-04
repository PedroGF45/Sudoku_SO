#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../utils/logs/logs-common.h"
#include "../../utils/parson/parson.h"
#include "../../utils/network/network.h"
#include "../logs/logs.h"
#include "server-statistics.h"


void saveRoomStatistics(int roomId, double elapsedTime) {
    FILE *file = fopen("room_stats.log", "a");  // Abre o ficheiro em modo de append
    if (file != NULL) {
        fprintf(file, "Sala %d - Tempo de resolução: %.2f segundos\n", roomId, elapsedTime);
        fclose(file);
    } else {
        printf("Erro ao abrir o ficheiro de estatísticas.\n");
    }
}

void updateGameStatistics(ServerConfig *config, int gameID, int elapsedTime, float accuracy) {

    FILE *file = fopen(config->gamePath, "r");

    // Ler o ficheiro JSON
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);
    if (file_content == NULL) {
        fclose(file);
        err_dump(config, gameID, 0, "memory allocation failed when updating statistics", MEMORY_ERROR);
        return;
    }

    fread(file_content, 1, file_size, file);
    file_content[file_size] = '\0';
    fclose(file);

    // Parse do JSON
    JSON_Value *root_value = json_parse_string(file_content);
    JSON_Object *root_object = json_value_get_object(root_value);
    free(file_content);

    // Get the "games" array
    JSON_Array *games_array = json_object_get_array(root_object, "games");

    // iterate over the games array
    for (int i = 0; i < json_array_get_count(games_array); i++) {

        JSON_Object *game_object = json_array_get_object(games_array, i);
        int currentID = (int)json_object_get_number(game_object, "id");

        // if the game is found
        if (currentID == gameID) {

            int previousTimeRecord = (int)json_object_get_number(game_object, "timeRecord");
            float previousAccuracyRecord = (float)json_object_get_number(game_object, "accuracyRecord");
            
            // if elapsed time is less than the current one store it
            if (elapsedTime < previousTimeRecord || previousTimeRecord == 0) {
                json_object_set_number(game_object, "timeRecord", elapsedTime);

                // write to log
                char logMessage[100];
                snprintf(logMessage, sizeof(logMessage), "Tempo recorde atualizado para %d segundos no jogo %d", elapsedTime, gameID);
                produceLog(config, logMessage, EVENT_NEW_RECORD, gameID, 0);

                printf("Tempo recorde atualizado de %d para %d segundos no jogo %d\n", previousTimeRecord, elapsedTime, gameID);
            }

            // if accuracy is greater than the current one store it
            if (accuracy > previousAccuracyRecord || previousAccuracyRecord == 0) {
                json_object_set_number(game_object, "accuracyRecord", accuracy);

                // write to log
                char logMessage[100];
                snprintf(logMessage, sizeof(logMessage), "Precisão recorde atualizada para %f no jogo %d", accuracy, gameID);
                produceLog(config, logMessage, EVENT_NEW_RECORD, gameID, 0);

                printf("Precisão recorde atualizada de %.2f para %.2f no jogo %d\n", previousAccuracyRecord, accuracy, gameID);
            }
            break;
        }
    }

    // write the updated JSON to the file
    file = fopen(config->gamePath, "w");
    if (file == NULL) {
        err_dump(config, gameID, 0, "can't open file to write updated statistics", MEMORY_ERROR);
        json_value_free(root_value);
        return;
    }

    char *serialized_string = json_serialize_to_string_pretty(root_value);
    fwrite(serialized_string, 1, strlen(serialized_string), file);
    fclose(file);
    free(serialized_string);

    // free the JSON object
    json_value_free(root_value);
}

void sendRoomStatistics(ServerConfig *config, Client *client) {
    FILE *file = fopen("room_stats.log", "r");
    if (file == NULL) {
        const char *errorMsg = "Erro: Não foi possível abrir o ficheiro de estatísticas.\n";
        if (send(client->socket_fd, errorMsg, strlen(errorMsg), 0) < 0) {
            // erro ao enviar mensagem de erro
            err_dump(config, 0, client->clientID, "can't send error message to client", EVENT_MESSAGE_SERVER_NOT_SENT);
        } else {
            printf("Erro: Não foi possível abrir o ficheiro de estatísticas\n");
            produceLog(config, "Erro: Não foi possível abrir o ficheiro de estatísticas", EVENT_MESSAGE_SERVER_SENT, 0, client->clientID);
        }
        return;
    }

    char line[256];
    char stats[1024] = "";  // Buffer para enviar todas as estatísticas
    while (fgets(line, sizeof(line), file) != NULL) {
        strncat(stats, line, sizeof(stats) - strlen(stats) - 1);  // Adiciona cada linha ao buffer
    }
    fclose(file);

    // if there are no statistics send "No statistics available"
    if (strlen(stats) == 0) {
        strcpy(stats, "No statistics available");
    }
        

    // Envia as estatísticas ao cliente
    if (send(client->socket_fd, stats, strlen(stats), 0) < 0) {
        // erro ao enviar estatísticas
        err_dump(config, 0, client->clientID, "can't send statistics to client", EVENT_MESSAGE_SERVER_NOT_SENT);
    } else {
        printf("Estatísticas enviadas ao cliente\n");
        produceLog(config, "Estatísticas enviadas ao cliente", EVENT_MESSAGE_SERVER_SENT, 0, client->clientID);
    }
}

