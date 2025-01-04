#ifndef CLIENT_MENUS_H
#define CLIENT_MENUS_H

#include <stdbool.h>
#include "../config/config.h"
#include "client-comms.h"

#define INTERFACE_MENU "1. Play\n2. Statistics\n3. Exit\nChoose an option: "
#define INTERFACE_PLAY_MENU "1. Singleplayer\n2. Multiplayer\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_SELECT_SINGLEPLAYER_GAME "1. New Random SinglepLayer Game\n2. New Specific Singleplayer Game\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_SELECT_MULTIPLAYER_GAME "1. New Random Multiplayer Game\n2. New Specific Multiplayer Game\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_SELECT_MULTIPLAYER_MENU "1. Create a New Multiplayer Game\n2. Join a Multiplayer Game\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_POSSIBLE_SYNCHRONIZATION "1. Readers-Writers\n2. Barber-Shop with static priority\n3. Barber-shop with dynamic priority\n4. Barber-Shop with FIFO\n5. Back\n6. Exit\nChoose an option: "

// Exibe o menu principal e processa as opções do utilizador.
void showMenu(int *socketfd, clientConfig *config);

// Exibe o menu de opções de jogo e processa as escolhas.
void showPlayMenu(int *socketfd, clientConfig *config);

// Exibe o menu para iniciar um novo jogo single player.
void showSinglePLayerMenu(int *socketfd, clientConfig *config);

// Exibe o menu de estatisticas
void showStatisticsMenu(int *socketfd, clientConfig *client);

// Exibe o menu de opções multiplayer.
void showMultiPlayerMenu(int *socketfd, clientConfig *config);

// Permite ao utilizador criar ou entrar num novo jogo multiplayer.
void createNewMultiplayerGame(int *socketfd, clientConfig *config);

// shows menu for possible synchronizations
void showPossibleSynchronizations(int *socketfd, clientConfig *config);

// Exibe o menu de opções multiplayer.
void showMultiPlayerMenu(int *socketfd, clientConfig *config);

// Solicita e exibe as salas multiplayer disponíveis.
void showMultiplayerRooms(int *socketfd, clientConfig *config);

// Exibe jogos disponíveis para o utilizador selecionar.
void showGames(int *socketfd, clientConfig *config, bool isSinglePlayer);

// Recebe um temporizador do servidor e atualiza o tempo restante.
void receiveTimer(int *socketfd, clientConfig *config);

// Inicia um jogo single player aleatório.
void playSinglePlayerGame(int *socketfd, clientConfig *config);

// Inicia um jogo multiplayer aleatório.
void playMultiPlayerGame(int *socketfd, clientConfig *config, char *synchronization);

// Função para mostrar timer update
int showTimerUpdate(char *buffer, int timeLeft);

#endif // CLIENT_MENUS_H