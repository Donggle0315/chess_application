
flags = -lpthread

server: server.o
	gcc -o server server.o $(flags) `mysql_config --libs`

server.o: server.h server.c
	gcc -o server.o -c server.c $(flags) `mysql_config --cflags`