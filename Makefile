all: clean Server Client

Server: server.o features.o
	g++ server.o features.o -o Server -lpthread -lssl -lcrypto

Client: client.o
	g++ client.o -o Client -lpthread

server.o: server.cpp features.h
	g++ -c -Wall -g -std=c++0x server.cpp -lssl -lcrypto

client.o: client.cpp
	g++ -c -Wall -g client.cpp

features.o: features.h
	g++ -c -Wall -g -std=c++0x features.cpp

clean:
	rm -f *.o Server Client
