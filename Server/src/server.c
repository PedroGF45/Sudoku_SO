#include <stdio.h>
#include "jogos.h"
#include "logs.h"

int main() {
    int idJogo = 1;  // ID do jogo que queremos carregar (deviamos receber isto como argumento)
    int idJogador = 2 ; // ID do jogador que queremos carregar (deviamos receber isto como argumento)

    // Load ao jogo com ID 1 a partir do ficheiro 'games.txt'
    /*Aqui estava com um erro no caminho relativo do ficheiro por isso usei o caminho absoluto
    // MODIFICAR O CAMINHO PARA O CAMINHO CORRETO NO VOSSO COMPUTADOR*/
    Jogo jogo = carregaJogo("Server/data/games.txt", idJogo);

    writeLog("Server/data/logs.txt", "Jogo carregado",idJogador);

    // Mostra a String do jogo incompleta (usar zeros para os espacos em branco)
    printf("Tabuleiro do Jogo %d: \n", idJogo);
    mostraTabuleiro(jogo);

    writeLog("Server/data/logs.txt", "Tabuleiro do jogo mostrado",idJogador);

    // Pergunta ao utilizador para inserir a solucao
    char solucaoEnviada[82];
    printf("Insira a sua solucao para o jogo %d: ", idJogo);
    scanf("%81s", solucaoEnviada);

    writeLog("Server/data/logs.txt", "Solucao introduzida",idJogador);

    // Compara a solucao introduzida com a solucao correta
    if (verificaSolucao(jogo, solucaoEnviada)) {
        printf("Solucao correta!\n");
        writeLog("Server/data/logs.txt", "Solucao correta",idJogador);
    } else {
        printf("Solucao errada.\n");
        writeLog("Server/data/logs.txt", "Solucao errada",idJogador);
    }

    return 0;
}
