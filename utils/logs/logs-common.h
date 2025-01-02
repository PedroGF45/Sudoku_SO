#ifndef LOGS_COMMON_H
#define LOGS_COMMON_H

/* server */
#define EVENT_SERVER_START                      "Server inicializado"
#define EVENT_GAME_LOAD                         "Jogo carregado"
#define EVENT_GAME_NOT_LOAD                     "Jogo nao carregado"
#define EVENT_GAME_NOT_FOUND                    "Jogo nao encontrado"
#define EVENT_BOARD_SHOW                        "Tabuleiro do jogo mostrado"
#define EVENT_SOLUTION_SENT                     "Solucao recebida"
#define EVENT_SOLUTION_CORRECT                  "Solucao correta"
#define EVENT_SOLUTION_INCORRECT                "Solucao errada"
#define EVENT_GAME_OVER                         "Jogo terminado"
#define EVENT_MESSAGE_SERVER_SENT               "Mensagem enviada para o cliente"
#define EVENT_MESSAGE_SERVER_NOT_SENT           "Erro ao enviar mensagem para o cliente"
#define EVENT_MESSAGE_SERVER_RECEIVED           "Mensagem recebida do cliente"
#define EVENT_MESSAGE_SERVER_NOT_RECEIVED       "Erro ao receber mensagem do cliente"
#define EVENT_CONNECTION_SERVER_ERROR           "Erro na conexao do servidor"
#define EVENT_CONNECTION_SERVER_ESTABLISHED     "Conexao estabelecida com o cliente"
#define EVENT_SERVER_THREAD_ERROR               "Erro ao criar thread"
#define EVENT_SERVER_CONNECTION_FINISH          "Conexao terminada com o cliente"
#define EVENT_SERVER_GAMES_SENT                 "Jogos enviados para o cliente"
#define EVENT_ROOM_NOT_CREATED                          "Sala nao criada"
#define EVENT_ROOM_LOAD                         "Sala carregada"
#define EVENT_ROOM_NOT_LOAD                     "Sala nao carregada"
#define EVENT_ROOM_JOIN                         "Jogador entrou na sala"
#define EVENT_ROOM_NOT_JOIN                     "Jogador nao entrou na sala"
#define EVENT_ROOM_DELETE                       "Sala eliminada"
#define EVENT_ROOM_NOT_DELETE                   "Sala nao eliminada"
#define EVENT_NEW_RECORD                        "Novo recorde"
#define EVENT_THREAD_NOT_CREATE                  "Erro ao criar a thread"
#define EVENT_BARBER_CREATED                    "Barbeiro criado"

/* client */
#define EVENT_CONNECTION_CLIENT_ESTABLISHED     "Conexao estabelecida com o servidor"
#define EVENT_CONNECTION_CLIENT_NOT_ESTABLISHED "Erro ao estabelecer conexao com o servidor"
#define EVENT_CONNECTION_CLIENT_CLOSED          "Conexao com o servidor fechada"
#define EVENT_MESSAGE_CLIENT_SENT               "Mensagem enviada para o servidor"
#define EVENT_MESSAGE_CLIENT_NOT_SENT           "Erro ao enviar mensagem para o servidor"
#define EVENT_MESSAGE_CLIENT_RECEIVED           "Mensagem recebida do servidor"
#define EVENT_MESSAGE_CLIENT_NOT_RECEIVED       "Erro ao receber mensagem do servidor"


#define MEMORY_ERROR                            "Erro de memoria"

#define BUFFER_SIZE 1024

struct log {
    int id;
    char event[32];
    char log[256];
};

// wirte log in JSON format
void writeLogJSON(const char *filename, int gameID, int playerID, const char *logMessage);

// concatenate info
char *concatenateInfo(char *msg, char* event, int idJogo, int idJogador);

#endif // LOGS_COMMON_H