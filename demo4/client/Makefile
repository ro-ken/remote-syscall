 cc = gcc

client : rsc_client.o client.o rsc_semaphore.o
	$(cc) rsc_client.o client.o rsc_semaphore.o -o client

rsc_client.o : rsc_client.c
	$(cc) -c $^ -o rsc_client.o

client.o : client.c
	$(cc) -c $^ -o client.o

rsc_semaphore.o : rsc_semaphore.c
	$(cc) -c $^ -o rsc_semaphore.o

clean :
	rm client rsc_client.o client.o rsc_semaphore.o

.PHONY: clean client