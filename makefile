CC = gcc
CFLAGS = -Wall

all: server # Adicionar o client quando estiver pronto

#client: client\client.c
#	gcc -o client client\client.c


# Compilar o servidor
server: Server/src/server.o Server/src/jogos.o Server/src/logs.o
	$(CC) $(CFLAGS) -o server Server/src/server.o Server/src/jogos.o Server/src/logs.o

# Compila server.c, jogos.c e logs.c para (.o)
Server/src/server.o: Server/src/server.c
	$(CC) $(CFLAGS) -c Server/src/server.c -o Server/src/server.o

Server/src/jogos.o: Server/src/jogos.c Server/src/jogos.h
	$(CC) $(CFLAGS) -c Server/src/jogos.c -o Server/src/jogos.o

Server/src/logs.o: Server/src/logs.c Server/src/logs.h
	$(CC) $(CFLAGS) -c Server/src/logs.c -o Server/src/logs.o

# Limpar os ficheiros .o e o executável
clean:
# Usar no linux para limpar (nao funciona no windows)
# 	rm -f Server/src/*.o server 
	del /Q Server\src\server.o Server\src\jogos.o  Server\src\logs.o server.exe