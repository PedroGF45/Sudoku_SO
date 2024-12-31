CC = gcc
CFLAGS = -g -c -Wall -Werror

# Paths
CLIENT_SRC = client/src
CLIENT_CONFIG = client/config
CLIENT_LOGS = client/logs
SERVER_SRC = server/src
SERVER_CONFIG = server/config
SERVER_LOGS = server/logs
UTILS_LOGS = utils/logs
UTILS_PARSON = utils/parson
UTILS_NETWORK = utils/network

# Object files for client, server, and utilities
CLIENT_OBJS = $(CLIENT_SRC)/client.o $(CLIENT_SRC)/client-comms.o $(CLIENT_SRC)/client-game.o $(CLIENT_CONFIG)/config.o $(CLIENT_LOGS)/logs.o
SERVER_OBJS = $(SERVER_SRC)/server.o $(SERVER_SRC)/server-comms.o $(SERVER_SRC)/server-game.o $(SERVER_CONFIG)/config.o $(SERVER_LOGS)/logs.o
UTIL_OBJS = $(UTILS_LOGS)/logs-common.o $(UTILS_PARSON)/parson.o $(UTILS_NETWORK)/network.o 

# Targets
all: server client

# Client build
client: $(CLIENT_OBJS) $(UTIL_OBJS)
	$(CC) -o client.exe $(CLIENT_OBJS) $(UTIL_OBJS)

# Server build
server: $(SERVER_OBJS) $(UTIL_OBJS)
	$(CC) -o server.exe $(SERVER_OBJS) $(UTIL_OBJS) -lpthread

# Compile client object files
$(CLIENT_SRC)/client.o: $(CLIENT_SRC)/client.c $(CLIENT_CONFIG)/config.h
	$(CC) $(CFLAGS) $(CLIENT_SRC)/client.c -o $@

$(CLIENT_SRC)/client-comms.o: $(CLIENT_SRC)/client-comms.c $(CLIENT_SRC)/client-comms.h
	$(CC) $(CFLAGS) $(CLIENT_SRC)/client-comms.c -o $@

$(CLIENT_SRC)/client-game.o: $(CLIENT_SRC)/client-game.c $(CLIENT_SRC)/client-game.h
	$(CC) $(CFLAGS) $(CLIENT_SRC)/client-game.c -o $@

$(CLIENT_CONFIG)/config.o: $(CLIENT_CONFIG)/config.c $(CLIENT_CONFIG)/config.h
	$(CC) $(CFLAGS) $(CLIENT_CONFIG)/config.c -o $@

$(CLIENT_LOGS)/logs.o: $(CLIENT_LOGS)/logs.c $(CLIENT_LOGS)/logs.h
	$(CC) $(CFLAGS) $(CLIENT_LOGS)/logs.c -o $@

# Compile server object files
$(SERVER_SRC)/server.o: $(SERVER_SRC)/server.c $(SERVER_CONFIG)/config.h $(SERVER_SRC)/server-comms.h $(SERVER_SRC)/server-game.h
	$(CC) $(CFLAGS) $(SERVER_SRC)/server.c -o $@

$(SERVER_SRC)/server-comms.o: $(SERVER_SRC)/server-comms.c $(SERVER_SRC)/server-comms.h
	$(CC) $(CFLAGS) $(SERVER_SRC)/server-comms.c -o $@

$(SERVER_SRC)/server-game.o: $(SERVER_SRC)/server-game.c $(SERVER_SRC)/server-game.h 
	$(CC) $(CFLAGS) $(SERVER_SRC)/server-game.c -o $@

$(SERVER_CONFIG)/config.o: $(SERVER_CONFIG)/config.c $(SERVER_CONFIG)/config.h 
	$(CC) $(CFLAGS) $(SERVER_CONFIG)/config.c -o $@

$(SERVER_LOGS)/logs.o: $(SERVER_LOGS)/logs.c $(SERVER_LOGS)/logs.h
	$(CC) $(CFLAGS) $(SERVER_LOGS)/logs.c -o $@

# Compile utility object files
$(UTILS_LOGS)/logs-common.o: $(UTILS_LOGS)/logs-common.c $(UTILS_LOGS)/logs-common.h 
	$(CC) $(CFLAGS) $(UTILS_LOGS)/logs-common.c -o $@

$(UTILS_PARSON)/parson.o: $(UTILS_PARSON)/parson.c $(UTILS_PARSON)/parson.h
	$(CC) $(CFLAGS) $(UTILS_PARSON)/parson.c -o $@

$(UTILS_NETWORK)/network.o: $(UTILS_NETWORK)/network.c $(UTILS_NETWORK)/network.h
	$(CC) $(CFLAGS) $(UTILS_NETWORK)/network.c -o $@

# Clean up
clean:
	rm -f $(SERVER_SRC)/*.o $(SERVER_CONFIG)/*.o $(SERVER_LOGS)/*.o server.exe $(CLIENT_SRC)/*.o $(CLIENT_CONFIG)/*.o $(CLIENT_LOGS)/*.o client.exe $(UTILS_LOGS)/*.o $(UTILS_PARSON)/*.o $(UTILS_NETWORK)/*.o
