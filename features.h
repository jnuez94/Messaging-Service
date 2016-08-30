#ifndef __FEATURES_H__
#define __FEATURES_H__

#include <string>
#include <vector>
#include <ctime>
#include <fstream>

class User {
public:
	User();
	User(std::string username, std::string password);
	std::string getUsername();
	std::string getPassword();
	int getSocket();
	void setSocket(int socket);
	std::time_t getLogtime();
	void setLogintime(std::time_t login_time);
  std::time_t getLastAct();
  void setLastAct(std::time_t last_act);
	bool isOnline();
	void Online();
	void Offline();
private:
	std::string username;
	std::string password;
	bool online;
	std::time_t login_time;
	int socket;
  std::time_t last_act;
};

class Blocked_Users{
private:
	std::string username;
	char* IP_address;
	time_t block_time;
public:
	Blocked_Users();
	Blocked_Users(std::string username, char* IP_address, time_t block_time);
  std::string getUsername();
  char* getIP();
	time_t getBlockTime();
};

std::vector<User> read_users(std::ifstream &is);

#endif
