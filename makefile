# Purpose: Makefile for the server and client programs
server: server.o
	gcc server.o -g -o server

server.o:
	gcc -c server\server.c

client: client\client.c
	gcc -o client client\client.c 

clean: 
	rm -rf *.o