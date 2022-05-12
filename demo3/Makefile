cc = gcc

all : rsc_server.o rsc_socket.o rsc_client.o
	$(cc) rsc_server.o rsc_socket.o -o server
	$(cc) rsc_client.o rsc_socket.o -o client

rsc_server.o : rsc_server.c
	$(cc) -c $^ -o rsc_server.o

rsc_socket.o : rsc_socket.c
	$(cc) -c $^ -o rsc_socket.o

rsc_client.o : rsc_client.c
	$(cc) -c $^ -o rsc_client.o

clean :
	rm server client rsc_server.o rsc_client.o rsc_socket.o

.PHONY: clean all