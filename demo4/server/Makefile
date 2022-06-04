 cc = gcc

server : rsc_server.o server.o
	$(cc) rsc_server.o server.o -o server

rsc_server.o : rsc_server.c
	$(cc) -c $^ -o rsc_server.o

server.o : server.c
	$(cc) -c $^ -o server.o

clean :
	rm server rsc_server.o server.o

.PHONY: clean client