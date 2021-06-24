.PHONY: server
.PHONY: clients

server: 
	./server 7777
clients:
	./clients 127.0.0.1 7777
clean:
	rm server
	rm clients
	./clearing
gcconly: server.c clients.c
	gcc server.c -o server
	gcc clients.c -o clients -lpthread -lrt
all:
	gcc server.c -o server
	gcc clients.c -o clients -lpthread -lrt
	./server 7777 &
	./clients 127.0.0.1 7777
	rm server
	rm clients
	./clearing
dokumentacia:
	more Dokumentacia.txt
