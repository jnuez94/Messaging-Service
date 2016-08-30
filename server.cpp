#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <time.h>

#include <pthread.h>
#include <algorithm>
#include <regex>
#include <openssl/sha.h>
#include <openssl/crypto.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include "features.h"

using namespace std;

#define MAXPENDING 5
#define BUFFSIZE 2048

// Times in seconds
#define BLOCK_TIME 1000
#define TIME_OUT 30

static void die(const char* message){
	perror(message);
	exit(1);
}

vector<User> users;
vector<Blocked_Users> blocked;


void logout(int client_socket){
  for(unsigned int i = 0; i < users.size(); i++){
    if(users[i].getSocket() == client_socket){
      users[i].Offline();
    }
  }
}

void server_message(int client_socket, const char* message){
  char server_response[BUFFSIZE];
  memset(&server_response, 0, sizeof(server_response));
  snprintf(server_response, sizeof(server_response), "%s", message);
  if(send(client_socket, server_response, strlen(server_response), 0) < 0){
    memset(&server_response, 0, sizeof(server_response));
    fprintf(stderr, "Client Disconnected\n");
    logout(client_socket);
    pthread_exit(NULL); 
  }
}

bool isBlocked(string username, char* ip_addr){
  if(blocked.empty())
    return false;
  for(unsigned int i = 0; i < blocked.size(); i++){
    time_t now;
    now = time(NULL);
    if((blocked[i].getUsername() == username) && strcmp(blocked[i].getIP(),ip_addr) == 0){
      if((now-blocked[i].getBlockTime()) < BLOCK_TIME){
        return true;
      }
      blocked.erase(blocked.begin()+i);
      return false;
    }
  }
  return false;
}

void who(int client_socket, vector<User> _users){
  string online_users;
	for(unsigned int i = 0; i < _users.size(); i++){
		if(_users[i].isOnline()){
      online_users = ":: " + _users[i].getUsername() + "\n";
			server_message(client_socket, online_users.c_str());
		}
	}
}



void broadcast(int client_socket, int user_index, string message){
  
  message.erase(0,10);
  string sender;
  sender = ":: " + users[user_index].getUsername() + " said " + message;
  
  for(unsigned int i = 0; i < users.size(); i++){
    if(users[i].isOnline()){
      if(users[i].getSocket() == client_socket){
        continue;
      }
    fprintf(stderr, "%s", sender.c_str());
    server_message(users[i].getSocket(), sender.c_str());
    }
  }
}

void last(int client_socket, string message){
  message.erase(0,5);
  stringstream iss;
  string temp;
  string _users;
  int range;
  iss << message;
  iss >> temp;
  range = atoi(temp.c_str());
  time_t now = time(NULL);
  for(unsigned int i = 0; i < users.size(); i++){
    fprintf(stderr, "User: %s ==> Log Time: %ld, Last Activity: %ld\n", users[i].getUsername().c_str(), users[i].getLogtime(), users[i].getLastAct()); 
    if(users[i].isOnline()){
      if((now-users[i].getLastAct()) < range*60){
         _users = ":: " + users[i].getUsername() + "\n";
         server_message(client_socket, _users.c_str()); 
      }
    }
    else{
      if((now-users[i].getLogtime()) < range*60){
        _users = ":: " + users[i].getUsername() + "\n";
        server_message(client_socket, _users.c_str());
      }
    }
  }
}

void send_message(int client_socket, int user_index, string message){
  message.erase(0,5);
  string sender;
  string receiver;
  stringstream iss;
  iss << message;
  iss >> receiver;
  message.erase(0,receiver.length()+1);
  sender = ":: " + users[user_index].getUsername() + " said " + message;
  
  for(unsigned int i = 0; i < users.size(); i++){
    if(users[i].getUsername() == receiver && users[i].isOnline())
      server_message(users[i].getSocket(), sender.c_str());
  }
}

void multiple_send(int client_socket, int user_index, string message){
  message.erase(0,6);
  size_t pos = message.find(")");
  string receivers = message.substr(0, pos);
  message.erase(0,pos+2);
  
  string sender;
  sender = ":: " + users[user_index].getUsername() + " said " + message;
  
  stringstream iss;
  iss << receivers;
  string r_users;
  while(iss >> r_users){
    for(unsigned int i = 0; i < users.size(); i++){
      if(users[i].getUsername() == r_users && users[i].isOnline())
        server_message(users[i].getSocket(), sender.c_str());
    }
  }
}

void commandHandler(int client_socket, int user_index){
	char client_response[BUFFSIZE];

  memset(&client_response, 0, sizeof(client_response));
  time_t activity;
  

	//start of loop
	for(;;){
		server_message(client_socket, ">> Command:\n");

		if(recv(client_socket, client_response, sizeof(client_response), 0) < 0)
			fprintf(stderr, "recv() failed on command");
      
    
		string command(client_response);
		memset(&client_response, 0, sizeof(client_response));
    time(&activity);
    users[user_index].setLastAct(activity);
    fprintf(stderr, "Last activity: %ld\n", users[user_index].getLastAct());

		if(regex_match(command, regex("(who)(.*)"))){
      who(client_socket, users);
		}
		else if(regex_match(command, regex("(last)(.*)"))){

      last(client_socket, command);
		}
		else if(regex_match(command, regex("(broadcast)(.*)"))){
      broadcast(client_socket, user_index, command);
		}
		else if(regex_match(command, regex("(send)(.*)"))){
   
      if(command.find("(") != string::npos){
        multiple_send(client_socket, user_index, command);
      }
      else{
        send_message(client_socket, user_index, command);
      }
		}
		else if(regex_match(command, regex("(logout)(.*)"))){
      users[user_index].Offline();
      server_message(client_socket, "Successful logout!\n");
      break;
		}
		command.clear();
	}
}

void convertSHA1BinaryToCharStr(const unsigned char * const hashbin, char * const hashstr) {
  for(int i = 0; i<20; ++i)
  {
    sprintf(&hashstr[i*2], "%02X", hashbin[i]);
  }
  for(int i = 0; i < 40; i++){
    hashstr[i] = tolower(hashstr[i]);
  }
  hashstr[40]=0;
}



int authenticate(int client_socket, int &try_count, char* IP_addr){

	char us[BUFFSIZE];
	char pass[BUFFSIZE];
  char server_resp[BUFFSIZE];
  unsigned char hash[SHA_DIGEST_LENGTH];
  char out[41];
  
  memset(&server_resp, 0, sizeof(server_resp));
  memset(&us, 0, sizeof(us));
  memset(&pass, 0, sizeof(pass));
  
  string password;
  server_message(client_socket, ">> Username: \n");
  if(recv(client_socket, us, sizeof(us), 0) < 0)
     fprintf(stderr, "recv() failed on username\n");
  string username(us);
  if(username.length() > 1)
    username.erase(std::remove(username.begin(), username.end(), '\n'), username.end());
  memset(&us, 0, sizeof(us));
  if(isBlocked(username, IP_addr)){
    server_message(client_socket, "User is blocked. Try again later.\n");
    return 0;
  }
  
  for(unsigned int i = 0; i < users.size(); i++){
    if(users[i].getUsername() == username && !users[i].isOnline()){
      while(try_count <= 3){
        ++try_count;
        server_message(client_socket, ">> Password: \n");
        if(recv(client_socket, pass, sizeof(pass), 0) < 0)
          fprintf(stderr, "recv() failed on password\n");
        
        string password(pass);
        if(password.length() > 1)
          password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());
        memset(&pass, 0, sizeof(pass));
        SHA1((unsigned char*)password.c_str(), password.length(), hash);
        convertSHA1BinaryToCharStr(hash, out);
        fprintf(stderr, "Hash: %s || Input Hash: %s\n", out, users[i].getPassword().c_str());
        if(strcmp((const char*)users[i].getPassword().c_str(), out) == 0){
          time_t login_time;
          time(&login_time);
          users[i].setLogintime(login_time);
          users[i].setLastAct(login_time);
          users[i].Online();
          users[i].setSocket(client_socket);
          return i+1;
        }
        
        if(try_count >= 3){
        	time_t blocked_time;
        	time(&blocked_time);
          fprintf(stderr,"Blocked Time: %ld\n", blocked_time);
        	blocked.push_back(Blocked_Users(username, IP_addr, blocked_time));
          server_message(client_socket, "User from this IP has been blocked\n");
        	return 0;
        }
        
        snprintf(server_resp, sizeof(server_resp), ">> Incorrect Password (Attempts remaining: %d)\n", 3-try_count);
        if(send(client_socket, server_resp, strlen(server_resp), 0) < 0){
          fprintf(stderr, "Client Disconnected");
        }
        memset(&server_resp, 0, sizeof(server_resp));
      }
    }
    else if(users[i].getUsername() == username && users[i].isOnline()){
      server_message(client_socket, "User is already logged in. Duplicate user login not supported!\n");
      return 0;
    }	
  }
  server_message(client_socket, "User does not exists!\n");
	return 0;
}

void* clientHandler(void* param){
	int client_socket = (long)param;
	char server_resp[BUFFSIZE];
	char client_resp[BUFFSIZE];
  memset(&server_resp, 0, sizeof(server_resp));
  memset(&client_resp, 0, sizeof(client_resp));
	struct sockaddr_in client_addr;
	socklen_t client_len;
	int user_index = 0;
	int try_count;

	client_len = sizeof(client_addr);
	if(getpeername(client_socket, (struct sockaddr*)&client_addr, &client_len) != 0)
		die("getpeername() failed");

	char * IP_addr = inet_ntoa(client_addr.sin_addr);

	while(1){ // Could've done while(true) but want to avoid infinite loop
    try_count = 0;
		user_index = authenticate(client_socket, try_count, IP_addr);

		if(user_index){
      fprintf(stderr, "Client %s connected from %s\n", users[user_index-1].getUsername().c_str(), IP_addr);
			server_message(client_socket, "Welcome to the Abyss Chat Server!\n");
			break;
		}
    

	}
  commandHandler(client_socket, user_index-1);

  logout(client_socket);
  close(client_socket);
	pthread_exit(NULL);
}

void * activity_tracker(void * param){
  for(;;){
    for(unsigned int i = 0; i < users.size(); i++){
      time_t curr_time = time(NULL);
      if(users[i].isOnline() && ((curr_time - users[i].getLastAct()) > TIME_OUT)){
        users[i].Offline();
        server_message(users[i].getSocket(), "You have been inactive for too long, thus you have been logged out\n");
        if(shutdown(users[i].getSocket(), 2)){
          die("shutdown() failed!");
          pthread_exit(NULL);
        }
      }
    }
  }
}

int main(int argc, char** argv){

	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		die("signal() failed");

	/* Variable declarations */
	int server_socket;
	int client_socket;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	unsigned short server_port;
	unsigned int client_len;

	/* Load username and passwords from user_pass.txt */
	ifstream fileIn("user_pass.txt");
	//std::vector<User>
  users = read_users(fileIn); 
	if(!users.empty())
    fprintf(stderr,"Users are loaded\n");
	fileIn.close();

	if(argc != 2){
		fprintf(stderr, "usage: %s <Server Port>\n", argv[0]);
		exit(1);
	}

	server_port = atoi(argv[1]);

	if((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		die("socket() failed");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(server_port);

	if(::bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		die("bind() failed");

	if(listen(server_socket, MAXPENDING) < 0)
		die("listen() failed");
   
  fprintf(stderr, "Server Online!\n");
  pthread_t tracker;
  pthread_create(&tracker, NULL, activity_tracker, NULL);
	for(;;){

		client_len = sizeof(client_addr);
		

		if((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len)) < 0)
			die("accept() failed");

		pthread_t tid;
		pthread_create(&tid, NULL, clientHandler, (void*)(long)client_socket);

	}

	return 0;
}

