#ifndef SERVER_STATISTICS_H
#define SERVER_STATISTICS_H

#include "../config/config.h"

// Guarda as estatísticas de uma sala no ficheiro 'room_stats.log'.
void saveRoomStatistics(int roomId, double elapsedTime);

// update game statistics
void updateGameStatistics(ServerConfig *config, int roomID, int elapsedTime, float accuracy);

// send message with statistics to client
void sendRoomStatistics(ServerConfig *config, Client *client);

#endif // SERVER_STATISTICS_H