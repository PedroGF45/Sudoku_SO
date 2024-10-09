#include <stdio.h>
#include "jogos.h"
#include "logs.h"

int main() {
    int idJogo = 1;  // ID do jogo que queremos carregar (deviamos receber isto como argumento)

    // Load ao jogo com ID 1 a partir do ficheiro 'games.txt'
    /*Aqui estava com um erro no caminho relativo do ficheiro por isso usei o caminho absoluto
    // MODIFICAR O CAMINHO PARA O CAMINHO CORRETO NO VOSSO COMPUTADOR*/
    Jogo jogo = carregaJogo("Server/data/games.txt", idJogo);

    writeLog("Server/data/logs.txt", "Jogo carregado");

    // Mostra a String do jogo incompleta (usar zeros para os espacos em branco)
    printf("Tabuleiro do Jogo %d: \n", idJogo);
    mostraTabuleiro(jogo);

    // Pergunta ao utilizador para inserir a solucao
    char solucaoEnviada[82];
    printf("Insira a sua solucao para o jogo %d: ", idJogo);
    scanf("%81s", solucaoEnviada);

    // Compara a solucao introduzida com a solucao correta
    if (verificaSolucao(jogo, solucaoEnviada)) {
        printf("Solucao correta!\n");
    } else {
        printf("Solucao errada.\n");
    }

    return 0;
}
