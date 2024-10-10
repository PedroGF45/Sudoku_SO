#include <stdlib.h>
#include <stdio.h>
#include "jogos.h"
#include "logs.h"
// Se argc(numero de argumentos que passamos) for menor que 3, isso significa que  não fornecemos
//argumentos suficientes (neste caso id_jogo e id_jogador)
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <id_jogo> <id_jogador>\n", argv[0]);
        return 1;
    }
    //Tivemos de usar a biblioteca stdlib.h para usar a função atoi
    // Converte o primeiro argumento para um inteiro e armazena em idJogo
    int idJogo = atoi(argv[1]);

    // Converte o segundo argumento para um inteiro e armazena em idJogador
    int idJogador = atoi(argv[2]);
    
    // Load ao jogo com ID 1 a partir do ficheiro 'games.txt'
    /*Aqui estava com um erro no caminho relativo do ficheiro por isso usei o caminho absoluto
    // MODIFICAR O CAMINHO PARA O CAMINHO CORRETO NO VOSSO COMPUTADOR*/
    Jogo jogo = carregaJogo("Server/data/games.txt", idJogo);

    writeLog("Server/data/logs.txt", idJogo ,idJogador, "Jogo carregado");

    // Mostra a String do jogo incompleta (usar zeros para os espacos em branco)
    printf("Tamanho tabuleiro do Jogo %d: \n", idJogo);
    mostraTabuleiro(jogo);

    writeLog("Server/data/logs.txt", idJogo ,idJogador, "Tabuleiro do jogo mostrado");

    // Pergunta ao utilizador para inserir a solucao
    char solucaoEnviada[82];
    printf("Insira a sua solucao para o jogo %d: ", idJogo);
    scanf("%81s", solucaoEnviada);

    writeLog("Server/data/logs.txt", idJogo ,idJogador, "Solucao introduzida");

    // Compara a solucao introduzida com a solucao correta
    if (verificaSolucao(jogo, solucaoEnviada)) {
        printf("Solucao correta!\n");
        writeLog("Server/data/logs.txt", idJogo ,idJogador, "Solucao correta");
    } else {
        printf("Solucao errada.\n");
        writeLog("Server/data/logs.txt", idJogo ,idJogador, "Solucao errada");
    }

    return 0;
}
