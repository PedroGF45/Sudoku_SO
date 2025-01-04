# Sudoku_SO
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out-server.txt ./server.exe server/config/server.conf
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out-client.txt ./client.exe client/config/client.conf