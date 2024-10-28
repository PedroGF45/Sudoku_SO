#ifndef LOGS_H
#define LOGS_H

/* server */
#define EVENT_SERVER_START                      "Server inicializado"
#define EVENT_GAME_LOAD                         "Jogo carregado"
#define EVENT_GAME_NOT_LOAD                     "Jogo nao carregado"
#define EVENT_BOARD_SHOW                        "Tabuleiro do jogo mostrado"
#define EVENT_SOLUTION_SENT                     "Solucao recebida"
#define EVENT_SOLUTION_CORRECT                  "Solucao correta"
#define EVENT_SOLUTION_WRONG                    "Solucao errada"
#define EVENT_GAME_OVER                         "Jogo terminado"
#define EVENT_MESSAGE_SERVER_SENT               "Mensagem enviada para o cliente"
#define EVENT_MESSAGE_SERVER_NOT_SENT           "Erro ao enviar mensagem para o cliente"
#define EVENT_MESSAGE_SERVER_RECEIVED           "Mensagem recebida do cliente"
#define EVENT_MESSAGE_SERVER_NOT_RECEIVED       "Erro ao receber mensagem do cliente"
#define EVENT_CONNECTION_SERVER_ERROR           "Erro na conexao do servidor"
#define EVENT_CONNECTION_SERVER_ESTABLISHED     "Conexao estabelecida com o cliente"

/* client */
#define EVENT_CONNECTION_CLIENT_ESTABLISHED     "Conexao estabelecida com o servidor"
#define EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED "Erro ao estabelecer conexao com o servidor"
#define EVENT_CONNECTION_CLIENT_CLOSED          "Conexao com o servidor fechada"
#define EVENT_MESSAGE_CLIENT_SENT               "Mensagem enviada para o servidor"
#define EVENT_MESSAGE_CLIENT_NOT_SENT           "Erro ao enviar mensagem para o servidor"
#define EVENT_MESSAGE_CLIENT_RECEIVED           "Mensagem recebida do servidor"
#define EVENT_MESSAGE_CLIENT_NOT_RECEIVED       "Erro ao receber mensagem do servidor"

struct log {
    int id;
    char event[32];
    char log[256];
};

// wirte log in JSON format
void writeLogJSON(const char *filename, int gameID, int playerID, const char *logMessage);

#endif