
flags = -lpthread

server: server.o room.o chess.o
	gcc -o server server.o room.o chess.o $(flags) `mysql_config --libs`

server.o: server.h server.c room.h
	gcc -o server.o -c server.c $(flags) `mysql_config --cflags`

room.o: room.h room.c chess.h
	gcc -o room.o -c room.c $(flags) `mysql_config --cflags`

chess.o: chess.h chess.c
	gcc -o chess.o -c chess.c $(flags) `mysql_config --cflags`