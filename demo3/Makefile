cc = gcc

run : test_server.o rsc_socket.o test_client.o
	$(cc) rsc_server.o rsc_socket.o -o server
	$(cc) rsc_client.o rsc_socket.o -o client

# client : rsc_client.o rsc_socket.o
# 	$(cc) rsc_client.o rsc_socket.o -o client

test_server.o : rsc_server.c
	$(cc) -c $^ -o rsc_server.o

rsc_socket.o : rsc_socket.c
	$(cc) -c $^ -o rsc_socket.o

test_client.o : test_client.c
	$(cc) -c $^ -o test_client.o

clean :
	rm testserver client rsc_server.o rsc_client.o rsc_socket.o

.PHONY: clean