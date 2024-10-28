#ifndef CLIENT_COMMS_H
#define CLIENT_COMMS_H

#include "../../utils/network/network.h"
#include "../config/config.h"

#define INTERFACE_MENU "1. Play\n2. Statistics\n3. Exit\nChoose an option: "
#define INTERFACE_PLAY_MENU "1. New Game\n2. Existing Game\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_SELECT_GAME "1. Random Game\n2. Specific Game\n3. Back\n4. Exit\nChoose an option: "

// connect to server
void connectToServer(struct sockaddr_in *serv_addr, int *socketfd, clientConfig config);

// show main menu
void showMenu(int *socketfd, clientConfig config);

// show play menu with games to select
void showPlayMenu(int *socketfd, clientConfig config);

// show new game menu with options to select
void showNewGameMenu(int *socketfd, clientConfig config);

// play random game
void playRandomGame(int *socketfd, clientConfig config);

// show statistics menu with statistics to show
//void showStatisticsMenu(int *socketfd, clientConfig config);

// show Sudoku board
void showBoard(char *buffer, char * logFileName, int gameID, int idJogador);

#endif