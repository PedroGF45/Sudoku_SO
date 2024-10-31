#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

int verifyLine(char *buffer);

void showBoard(char *buffer, char * logFileName, int playerID);

void getRandomLine(char *buffer);

#endif