CC = gcc
CFLAGS = -g -c

all: server client

# compilar o cliente
client: client/src/client.o client/config/config.o utils/logs/logs.o utils/parson/parson.o utils/network/network.o
	$(CC) -o client.exe client/src/client.o client/config/config.o utils/logs/logs.o utils/parson/parson.o utils/network/network.o

# compila o client.c e confic.c
client/src/client.o: client/src/client.c
	$(CC) $(CFLAGS) client/src/client.c -o client/src/client.o

client/config/config.o: client/config/config.c client/config/config.h
	$(CC) $(CFLAGS) client/config/config.c -o client/config/config.o

# Compilar o servidor
server: server/src/server.o server/src/jogos.o server/config/config.o utils/logs/logs.o utils/parson/parson.o utils/network/network.o
	$(CC) -o server.exe server/src/server.o server/src/jogos.o server/config/config.o utils/logs/logs.o utils/parson/parson.o utils/network/network.o

# Compila server.c, jogos.c logs.c e config.c para (.o)
server/src/server.o: server/src/server.c
	$(CC) $(CFLAGS) server/src/server.c -o server/src/server.o

server/src/jogos.o: server/src/jogos.c server/src/jogos.h 
	$(CC) $(CFLAGS) server/src/jogos.c -o server/src/jogos.o

server/config/config.o: server/config/config.c server/config/config.h 
	$(CC) $(CFLAGS) server/config/config.c -o server/config/config.o

# compila funções utilitárias
utils/logs/logs.o: utils/logs/logs.c utils/logs/logs.h 
	$(CC) $(CFLAGS) utils/logs/logs.c -o utils/logs/logs.o

utils/parson/parson.o: utils/parson/parson.c utils/parson/parson.h
	$(CC) $(CFLAGS) utils/parson/parson.c -o utils/parson/parson.o

utils/network/network.o: utils/network/network.c utils/network/network.h
	$(CC) $(CFLAGS) utils/network/network.c -o utils/network/network.o

# Limpar os ficheiros .o e os executáveis
clean:
	rm -f server/src/*.o server/config/*.o server.exe client/src/*.o client/config/*.o client.exe utils/logs/*.o utils/parson/*.o utils/network/*.o
#	del /Q server\src\server.o server\src\jogos.o  server\src\logs.o server\config\config.o server.exe parson.o