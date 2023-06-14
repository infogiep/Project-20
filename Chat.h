#pragma once
#include <vector>
#include <exception>
#include <memory>
#include "Message.h"
#include <Winsock2.h>
#include "User.h"
using namespace std;

struct UserLoginExp : public std::exception
{
	const char* what() const noexcept override { return "error: user login is busy"; }
};

struct UserNameExp : public std::exception
{
	const char* what() const noexcept override { return "error: user name is busy"; }
};

class Chat {
	bool isChatWork_ = false; // exit
	std::vector<User> users_; // users
	std::vector<Message> messages_; // messages
	std::shared_ptr<User> currentUser_ = nullptr; // current user

	void login();// enter to chat
	void signUp(); // reg to chat
	void showChat() const; // show messages
	void showAllUsersName() const; // list of users
	void addMessage(); // message
	std::vector<User>& getAllUsers() { return users_; }
	std::vector<Message>& getAllMessage() { return messages_; }
	std::shared_ptr<User> getUserByLogin(const std::string& login) const;
	std::shared_ptr<User> getUserByName(const std::string& name) const;

public:
	Chat() = default;
	Chat(const char* serverIp, int serverPort, int bufferLength);
	~Chat();
	void start();// start chat
	bool isChatWork() const { return isChatWork_; } // chat work? ( true - work, false - no)
	std::shared_ptr<User> getCurrentUser() const { return currentUser_; }
	void showLoginMenu(); // start menu
	void showUserMenu(); // menu
	void initSocket();               // initialisation socket
	void receiveMessage();           // receive mess
	void sendMessage(const char* message);  // send mess
private:
	WSADATA wsaData;                 // ver socket
	SOCKET udpSocket;                // UDP-socket
	sockaddr_in serverAddr;          // address socket
	sockaddr_in clientAddr;          // address client
	int clientAddrLen;               // length address client
	const char* serverIp;            // IP-address 
	const int serverPort;            // port server
	const int bufferLength;          // length buff
	char* receiveBuffer;             // buffer for receive data
	char* sendBuffer;                // buffer for send data
};