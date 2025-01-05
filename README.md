# Sudoku_SO
To compile the project:  
make all  
make server (compile server only)  
make client (compile client only)  

To start the server:  
./server.exe server/config/server.conf  

Server Variables (server.conf):  
-SERVER_PORT - port on where the server gets hosted  
-GAME_PATH - path for storing sudoku games (leave this by default)  
-SERVER_LOG_PATH - path for logging (leave this by default)  
-MAX_ROOMS - maximum number of rooms that can be created   
-MAX_PLAYERS_PER_ROOM - maximum number of players in each room created  
-MAX_PLAYERS_ON_SERVER - maximum number of players that can connect to the server  
-MAX_WAITING_TIME - maximum time that a player waits on queue (for barber shop with dynamic priorities)  
  
To start the client:  
./client.exe client/config/client.conf  

Client Variables (client.conf):  
SERVER_IP - ip where to connect to the server  
SERVER_PORT - port where to connect to the server (same as in server.conf)  
SERVER_HOSTNAME - hostname to be translated (not yet implemented)  
LOG_PATH - path for client logging (leave this by default)  
IS_MANUAL - switch to make client solve sudoku manually or automatically (0/1)  
IS_PREMIUM - switch to make client premium or not premium (0/1)  
DIFFICULTY = expertise of client (1-easy, 2-normal, 3-hard)  

Known bugs:  
Check if client/data exists. If not create data inside client.  

Check memory leaks:  
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out-server.txt ./server.exe server/config/server.conf  
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out-client.txt ./client.exe client/config/client.conf  
