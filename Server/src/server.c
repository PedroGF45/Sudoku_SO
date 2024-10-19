#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/logs/logs.h"
#include "../../utils/parson/parson.h"
#include "../config/config.h"
#include "jogos.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Erro: Faltam argumentos de configuracao!\n");
        return 1;
    }

    // Carrega a configuracao do servidor
    ServerConfig config = getServerConfig(argv[1]);

    // Define idJogo e idJogador
     /*Aqui depois temos de definir valores incrementados para cada jogo e jogador
    ou um random para atribuir um tabuleiro aleatorio ao jogador*/
    int idJogo = 2;
    int idJogador = 2;

    // Load ao jogo
    Jogo jogo = carregaJogo(config, idJogo, idJogador);

    // Mostra o tabuleiro
    printf("Tabuleiro inicial:\n");
    mostraTabuleiro(config.logPath, jogo, idJogador);

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
                printf("A linha deve conter exatamente 9 digitos.\n");
                // Pede novamente a linha se nao tiver 9 digitos
                continue;
            }

            // Preenche a linhaInserida e prepara valoresInseridos para o log
            for (int j = 0; j < 9; j++) {
                if (linha[j] >= '0' && linha[j] <= '9') {
                    // Converte char para int
                    linhaInserida[j] = linha[j] - '0';  
                } else {
                    linhaInserida[j] = 0;  // Coloca 0 para caracteres não numéricos
                }

                 // Adicionar o valor inserido a string
                char valor[3];
                // Preserva o caractere original
                sprintf(valor, "%c", linha[j]);  
                strcat(valoresInseridos, valor);
                if (j < 8) strcat(valoresInseridos, " ");
            }

            // Verifica a linha inserida
            linhaCorreta = verificaLinha(config.logPath, valoresInseridos, &jogo, linhaInserida, i, idJogador);

            char logMessage[100];
            // Escreve no log se a linha está correta ou errada
            if (!linhaCorreta) {
                printf("A linha %d contém erros ou esta incompleta.\n", i + 1);
                sprintf(logMessage, "%s errada para a linha %d: %s", EVENT_SOLUTION_WRONG, i + 1, valoresInseridos);
            } else {
                sprintf(logMessage, "%s para a linha %d: %s", EVENT_SOLUTION_CORRECT, i + 1, valoresInseridos);
            }
            writeLogJSON(config.logPath, idJogo, idJogador, logMessage);  // Log com a mensagem atualizada

            // Mostra o tabuleiro atualizado
            printf("Tabuleiro atualizado:\n");
            mostraTabuleiro(config.logPath, jogo, idJogador);
        }
    }

    return 0;
}
