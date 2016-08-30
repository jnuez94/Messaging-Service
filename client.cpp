#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>

#define BUFFSIZE 2048

static void die(const char* message){
	perror(message);
	exit(1);
}


int main(int argc, char** argv){

	int sock;								// Socket descriptor

	struct sockaddr_in server_addr;			// Server address
	unsigned short server_port;				// Server port
	char* serverIP;							// Server IP

	//unsigned int messagelen;				// Command length

	if(argc != 3){
		fprintf(stderr, "usage: %s <Server IP> <Server Port>\n", argv[0]);
		exit(1);
	}

	struct hostent *he;
	if((he = gethostbyname(argv[1])) == NULL)
		die("gethostbyname() failed");

	serverIP = inet_ntoa(*(struct in_addr*)he->h_addr);
	server_port = atoi(argv[2]);

	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		die("socket() failed");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(serverIP);
	server_addr.sin_port = htons(server_port);

	if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		die("connect() to server failed");

	fd_set fd_read, fd_write;
	char server_response[BUFFSIZE];
	char client_response[BUFFSIZE];
	memset(&server_response, 0, sizeof(server_response));
	memset(&client_response, 0, sizeof(client_response));
	int bytes_read;

	/* using select for nonblocking I/O */
	for(;;){
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_SET(sock, &fd_read);		
		FD_SET(STDIN_FILENO, &fd_read);
    	FD_SET(sock, &fd_write);

		if(select(sock+1, &fd_read, NULL, NULL, NULL) == -1){
			die("select() failed");
		}

		// Checks if server sent data 
    	if(FD_ISSET(sock, &fd_read)){
			if((bytes_read = recv(sock, server_response, sizeof(server_response), 0)) <= 0){
				if(bytes_read < 0)
					die("recv() failed");

				printf(">> Server Disconnected\n>> Client closing\n");
				close(sock);
				FD_CLR(sock, &fd_read);
				FD_CLR(sock, &fd_write);
				FD_CLR(STDIN_FILENO, &fd_read);
				break;
			}
			printf("%s", server_response);
			memset(&server_response, 0, sizeof(server_response));
		}
		// Checks if there is a keyboard input
		else if(FD_ISSET(STDIN_FILENO, &fd_read)){
			if(fgets(client_response, sizeof(client_response), stdin) == NULL)
				fprintf(stderr,"Message not sent");
      		if(FD_ISSET(sock, &fd_write)){
				if(send(sock, client_response, sizeof(client_response), 0) < 0)
					die("send() failed");
			}
			memset(&client_response, 0, sizeof(client_response));
		}

	}
	FD_CLR(sock, &fd_read);
	FD_CLR(sock, &fd_write);
	FD_CLR(STDIN_FILENO, &fd_read);

	close(sock);
	return 0;
}
