CC = gcc
CFLAGS = -g -c

all: server client utils# Adicionar o client quando estiver pronto

client: client/src/client.o client/config/config.o utils/utils.o parson.o
	$(CC) -o client.exe client/src/client.o client/config/config.o utils/utils.o parson.o

client/src/client.o: client/src/client.c
	$(CC) $(CFLAGS) client/src/client.c -o client/src/client.o

# Compilar o servidor
server: server/src/server.o server/src/jogos.o server/config/config.o utils/utils.o parson.o
	$(CC) -o server.exe server/src/server.o server/src/jogos.o server/config/config.o utils/utils.o parson.o

# Compila server.c, jogos.c e logs.c para (.o)
server/src/server.o: server/src/server.c
	$(CC) $(CFLAGS) server/src/server.c -o server/src/server.o

server/src/jogos.o: server/src/jogos.c server/src/jogos.h
	$(CC) $(CFLAGS) server/src/jogos.c -o server/src/jogos.o

# Compila config.c para (.o)
server/config/config.o: server/config/config.c server/config/config.h
	$(CC) $(CFLAGS) server/config/config.c -o server/config/config.o

client/config/config.o: client/config/config.c client/config/config.h
	$(CC) $(CFLAGS) client/config/config.c -o client/config/config.o

utils/utils.o: utils/logs/logs.c utils/logs/logs.h
	$(CC) $(CFLAGS) utils/logs/logs.c -o utils/utils.o

parson.o: parson.c parson.h
	$(CC) $(CFLAGS) parson.c -o parson.o
# Limpar os ficheiros .o e o executável
clean:
#	rm -f server/src/*.o server/config/config*.o server.exe client/src/*.o client/config/config*.o client.exe utils/*.o parson.o
	del /Q server\src\server.o server\src\jogos.o  server\src\logs.o server\config\config.o server.exe parson.o