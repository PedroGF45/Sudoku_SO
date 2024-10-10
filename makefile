CC = gcc
CFLAGS = -g -c

all: server # Adicionar o client quando estiver pronto

#client: client\client.c
#	gcc -o client client\client.c

# Compilar o servidor
server: Server/src/server.o Server/src/jogos.o Server/src/logs.o
	$(CC) -lpthread -o server.exe Server/src/server.o Server/src/jogos.o Server/src/logs.o

# Compila server.c, jogos.c e logs.c para (.o)
Server/src/server.o: Server/src/server.c
	$(CC) $(CFLAGS) Server/src/server.c -o Server/src/server.o

Server/src/jogos.o: Server/src/jogos.c Server/src/jogos.h
	$(CC) $(CFLAGS) Server/src/jogos.c -o Server/src/jogos.o

Server/src/logs.o: Server/src/logs.c Server/src/logs.h
	$(CC) $(CFLAGS) Server/src/logs.c -o Server/src/logs.o

# Limpar os ficheiros .o e o executável
clean:
	rm -f Server/src/*.o server.exe
#	del /Q Server\src\server.o Server\src\jogos.o  Server\src\logs.o server.exe