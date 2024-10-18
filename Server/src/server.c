#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jogos.h"
#include "../../utils/logs/logs.h"
#include "../config/config.h"
#include "C:\\Users\\claud\\OneDrive\\Documentos\\GitHub\\Sudoku_SO\\parson.h"

//get from config file


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuracao!\n");
        return 1;
    }

    // Carrega a configuracao do servidor
    serverConfig config = getServerConfig(argv[1]);

    // Debug: Verifica o caminho do ficheiro de log
    printf("PATH DO LOG: %s\n", config.logPath);

    // Define idJogo e idJogador
    /*Aqui depois temos de definir valores incrementados para cada jogo e jogador
    ou um random para atribuir um tabuleiro aleatorio ao jogador*/ 
    int idJogo = 1;
    int idJogador = 2;

    // Load ao jogo
    Jogo jogo = carregaJogo(config.gamePath, idJogo);

    writeLogJSON(config.logPath, idJogo, idJogador, "Jogo carregado");

    // Mostra o tabuleiro
    printf("Tabuleiro inicial:\n");
    mostraTabuleiro(jogo);

    // Perguntar pela solução linha por linha
    int linhaInserida[9];
    // Armazena cada linha da solução como string
    char linha[10];  
    // Armazena os valores inseridos pelo utilizador
    char valoresInseridos[50];  

    for (int i = 0; i < 9; i++) {
        // Controla se a linha esta correta
        int linhaCorreta = 0;  
        while (!linhaCorreta) {
            printf("Insira valores para a linha %d do tabuleiro (exactamente 9 digitos):\n", i + 1);
            // Ler a linha como uma string de no máximo 9 dígitos
            scanf("%9s", linha);  

            // Limpa a string de valores inseridos
            strcpy(valoresInseridos, "");

            // Verifica o tamanho da entrada do utilizador
            int len = strlen(linha);
            if (len != 9) {
                printf("A linha deve conter exatamente 9 dígitos.\n");
                // Pede novamente a linha se nao tiver 9 digitos
                continue;  
            }

            // Converter a linha de string para numeros e criar a string de valores inseridos
            for (int j = 0; j < 9; j++) {
                // Converter char para int
                linhaInserida[j] = linha[j] - '0';  

                // Adicionar o valor inserido a string
                char valor[3];
                sprintf(valor, "%d", linhaInserida[j]);
                strcat(valoresInseridos, valor);
                if (j < 8) strcat(valoresInseridos, " ");
            }

            // Escreve no log que a solucaoo foi enviada
            char logMessage[100];
            sprintf(logMessage, "Solucao enviada para a linha %d: %s", i + 1, valoresInseridos);
            writeLogJSON(config.logPath, idJogo, idJogador, logMessage);

            // Verifica a linha inserida
            linhaCorreta = verificaLinha(&jogo, linhaInserida, i);

            // Mostra o tabuleiro atualizado
            printf("Tabuleiro atualizado:\n");
            mostraTabuleiro(jogo);

            // Escreve no log se a linha está correta ou errada
            if (!linhaCorreta) {
                printf("A linha %d contem erros ou esta imcompleta.\n", i + 1);
                sprintf(logMessage, "Solucao errada para a linha %d: %s", i + 1, valoresInseridos);
            } else {
                sprintf(logMessage, "Solucao correta para a linha %d: %s", i + 1, valoresInseridos);
            }
            writeLogJSON(config.logPath, idJogo, idJogador, logMessage);  // Log com a mensagem atualizada
        }
    }

    return 0;
}

