#include "features.h"

/* User class methods */
User::User() : username(""), password(""), online(false), login_time(0), last_act(0) {}

User::User(std::string username, std::string password) : username(std::move(username)), password(std::move(password)), online(false), login_time(0), last_act(0) {}

std::string User::getUsername() {
	return username;
}

std::string User::getPassword() {
	return password;
}

int User::getSocket(){
	return socket;
}

void User::setSocket(int socket){
	this->socket = socket;
}

std::time_t User::getLogtime(){
	return login_time;
}

void User::setLogintime(std::time_t login_time){
	this->login_time = login_time;
}

std::time_t User::getLastAct(){
  return last_act;
}
void User::setLastAct(std::time_t last_act){
  this->last_act = last_act;
}

bool User::isOnline(){
	return online;
}

void User::Online(){
	online = true;
}

void User::Offline(){
	online = false;
}

/* Blocked_Users class method */

Blocked_Users::Blocked_Users() : username(""), IP_address(NULL), block_time(0) {}
Blocked_Users::Blocked_Users(std::string username, char* IP_address, time_t block_time) : username(std::move(username)), IP_address(IP_address), block_time(block_time) {}
std::string Blocked_Users::getUsername(){
  return username;
}
char* Blocked_Users::getIP(){
  return IP_address;
}
time_t Blocked_Users::getBlockTime(){
	return block_time;
}

/* Utility functions */
// Reads the userdata from the file
std::vector<User> read_users(std::ifstream &is){
	std::vector<User> results;
	std::string username, password;

	while(is >> username >> password){
		results.push_back(User(username, password));
	}

	return results;
} 
