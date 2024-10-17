#include <stdlib.h>
#include <stdio.h>
#include "jogos.h"
#include "logs.h"
#include "../config/config.h"


//get from config file

// Se argc(numero de argumentos que passamos) for menor que 3, isso significa que  não fornecemos
int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuração!\n");
        return 1;
    }

    // Carrega a configuracao do servidor
    serverConfig config = getServerConfig(argv[1]);

    // Tivemos de usar a biblioteca stdlib.h para usar a função atoi
    // Converte o primeiro argumento para um inteiro e armazena em idJogo
    int idJogo = 1;

    // Converte o segundo argumento para um inteiro e armazena em idJogador
    int idJogador = 2;
    
    // Load ao jogo com ID 1 a partir do ficheiro 'games.txt'
    Jogo jogo = carregaJogo(config.gamePath, idJogo);

    writeLog(config.logPath, idJogo ,idJogador, EVENT_GAME_LOAD);

    // Mostra a String do jogo incompleta (usar zeros para os espacos em branco)
    printf("Tamanho tabuleiro do Jogo %d: \n", idJogo);
    mostraTabuleiro(jogo);

    writeLog(config.logPath, idJogo ,idJogador, EVENT_BOARD_SHOW);

    // Pergunta ao utilizador para inserir a solucao
    char solucaoEnviada[82];
    printf("Insira a sua solucao para o jogo %d: ", idJogo);
    scanf("%81s", solucaoEnviada);

    writeLog(config.logPath, idJogo ,idJogador, EVENT_SOLUTION_SENT);

    // Compara a solucao introduzida com a solucao correta
    if (verificaSolucao(jogo, solucaoEnviada)) {
        printf("Solucao correta!\n");
        writeLog(config.logPath, idJogo ,idJogador, EVENT_SOLUTION_CORRECT);
    } else {
        printf("Solucao errada.\n");
        writeLog(config.logPath, idJogo ,idJogador, EVENT_SOLUTION_WRONG);
    }

    return 0;
}
