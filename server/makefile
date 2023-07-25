
flags = -lpthread -I/opt/homebrew/Cellar/mysql/8.0.33_3/include

server: server.o room.o chess.o
	gcc -o server server.o room.o chess.o $(flags) `mysql_config --libs`

server.o: server.h server.c
	gcc -o server.o -c server.c $(flags) `mysql_config --cflags`

room.o: room.h room.c
	gcc -o room.o -c room.c $(flags) `mysql_config --cflags`

chess.o: chess.h chess.c
	gcc -o chess.o -c chess.c $(flags) `mysql_config --cflags`