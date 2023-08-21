CC = g++ -Wno-write-strings
CLIENT_FILE = simple-client.c
SERVER_FILE = simple_server.cpp
HTTP_SERVER_FILE = http_server.cpp

all: client server

client: $(CLIENT_FILE)
	gcc $(CLIENT_FILE) -o client
	
server: $(SERVER_FILE) $(HTTP_SERVER_FILE)
	$(CC) $(SERVER_FILE) $(HTTP_SERVER_FILE) -o server -lpthread 
	
run: client server
	./server 8000

clean:
	rm -f server client
