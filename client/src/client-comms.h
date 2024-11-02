#ifndef CLIENT_COMMS_H
#define CLIENT_COMMS_H

#include <stdbool.h>
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "client-game.h"

#define INTERFACE_MENU "1. Play\n2. Statistics\n3. Exit\nChoose an option: "
#define INTERFACE_PLAY_MENU "1. Singleplayer\n2. Multiplayer\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_SELECT_GAME "1. New Random Game\n2. New Specific Game\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_SELECT_MULTIPLAYER_GAME "1. Create a New Multiplayer Game\n2. Join a Multiplayer Game\n3. Back\n4. Exit\nChoose an option: "

// connect to server
void connectToServer(struct sockaddr_in *serv_addr, int *socketfd, clientConfig config);

// show main menu
void showMenu(int *socketfd, clientConfig config);

// show play menu with games to select
void showPlayMenu(int *socketfd, clientConfig config);

// show new game menu with options to select
void showSinglePLayerMenu(int *socketfd, clientConfig config);

// play random game
void playRandomSinglePlayerGame(int *socketfd, clientConfig config);

// check existing games
void checkExistingGames(int *socketfd, clientConfig config);

// show multiplayer menu with options to select
void showMultiPlayerMenu(int *socketfd, clientConfig config);

// create new multiplayer game
void createNewMultiplayerGame(int *socketfd, clientConfig config);

// play random multiplayer game
void playRandomMultiPlayerGame(int *socketfd, clientConfig config);

// show multiplayer menu with options to select
void showMultiPlayerMenu(int *socketfd, clientConfig config);

// show multiplayer rooms
void showMultiplayerRooms(int *socketfd, clientConfig config);

// show statistics menu with statistics to show
//void showStatisticsMenu(int *socketfd, clientConfig config);

// send lines to server
void sendLines(int *socketfd, clientConfig config);

// show games to select
void showGames(int *socketfd, clientConfig config, bool isSinglePlayer);

// close connection
void closeConnection(int *socketfd, clientConfig config);

#endif