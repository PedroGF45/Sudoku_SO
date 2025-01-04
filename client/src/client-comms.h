#ifndef CLIENT_COMMS_H
#define CLIENT_COMMS_H

#include <stdbool.h>
#include "../../utils/network/network.h"
#include "../config/config.h"
#include "client-game.h"

#define INTERFACE_MENU "1. Play\n2. Statistics\n3. Exit\nChoose an option: "
#define INTERFACE_PLAY_MENU "1. Singleplayer\n2. Multiplayer\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_SELECT_SINGLEPLAYER_GAME "1. New Random SinglepLayer Game\n2. New Specific Singleplayer Game\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_SELECT_MULTIPLAYER_GAME "1. New Random Multiplayer Game\n2. New Specific Multiplayer Game\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_SELECT_MULTIPLAYER_MENU "1. Create a New Multiplayer Game\n2. Join a Multiplayer Game\n3. Back\n4. Exit\nChoose an option: "
#define INTERFACE_POSSIBLE_SYNCHRONIZATION "1. Readers-Writers\n2. Barber-Shop with static priority\n3. Barber-shop with dynamic priority\n4. Barber-Shop with FIFO\n5. Back\n6. Exit\nChoose an option: "


void showStatisticsMenu(int *socketfd);

// Estabelece uma ligação TCP com o servidor.
void connectToServer(struct sockaddr_in *serv_addr, int *socketfd, clientConfig *config);

// Exibe o menu principal e processa as opções do utilizador.
void showMenu(int *socketfd, clientConfig *config);

// Exibe o menu de opções de jogo e processa as escolhas.
void showPlayMenu(int *socketfd, clientConfig *config);

// Exibe o menu para iniciar um novo jogo single player.
void showSinglePLayerMenu(int *socketfd, clientConfig *config);

// Inicia um jogo single player aleatório.
void playSinglePlayerGame(int *socketfd, clientConfig *config);

// Exibe o menu de opções multiplayer.
void showMultiPlayerMenu(int *socketfd, clientConfig *config);

// Permite ao utilizador criar ou entrar num novo jogo multiplayer.
void createNewMultiplayerGame(int *socketfd, clientConfig *config);

void showPossibleSynchronizations(int *socketfd, clientConfig *config);

// Inicia um jogo multiplayer aleatório.
void playMultiPlayerGame(int *socketfd, clientConfig *config, char *synchronization);

// Exibe o menu de opções multiplayer.
void showMultiPlayerMenu(int *socketfd, clientConfig *config);

// Solicita e exibe as salas multiplayer disponíveis.
void showMultiplayerRooms(int *socketfd, clientConfig *config);

// Envia linhas de jogo ao servidor e processa o tabuleiro atualizado.
void playGame(int *socketfd, clientConfig *config);

// Exibe jogos disponíveis para o utilizador selecionar.
void showGames(int *socketfd, clientConfig *config, bool isSinglePlayer);

// Envia uma mensagem para fechar a conexão com o servidor.
void closeConnection(int *socketfd, clientConfig *config);

// Exibe o tabuleiro de jogo recebido do servidor.
char *showBoard(int *socketfd, clientConfig *config);

// Recebe um temporizador do servidor e atualiza o tempo restante.
void receiveTimer(int *socketfd, clientConfig *config);

// Acaba o jogo
void finishGame(int *socketfd, clientConfig *config, EstatisticasLinha *estatisticas);

#endif // CLIENT_COMMS_H