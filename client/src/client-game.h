#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

int verifyLine(char *buffer);

void showBoard(char *buffer, char *logFileName, int playerID);

void resolveLine(char *buffer, char *line, int row, int difficulty);

bool isValid(JSON_Array *board_array, int row, int col, int num, int difficulty);

#endif