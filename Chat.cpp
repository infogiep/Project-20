#include <iostream>
#include "Chat.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <cstring>

void Chat::start() {
	isChatWork_ = true; // enter to menu
}

std::shared_ptr<User> Chat::getUserByLogin(const std::string& login) const
{
	for (auto& user : users_) // for all users
	{
		if (login == user.getUserLogin()) // checking login
			return std::make_shared<User>(user); // pointer for user
	}
	return nullptr;
}

std::shared_ptr<User> Chat::getUserByName(const std::string& name) const
{
	for (auto& user : users_)
	{
		if (name == user.getUserName())
			return std::make_shared<User>(user); // pointer for user name
	}
	return nullptr;
}

void Chat::login() // login 
{
	std::string login, password;
	char operation;

	do
	{
		std::cout << "Login: ";
		std::cin >> login;
		std::cout << "Password: ";
		std::cin >> password;

		currentUser_ = getUserByLogin(login);

		if (currentUser_ == nullptr || (password != currentUser_->getUserPassword()))
		{
			currentUser_ = nullptr;
			std::cout << "Error" << std::endl;
			std::cout << "For exit (0) : ";
			std::cin >> operation;

			if (operation == '0')
				break;
		}
	} while (!currentUser_);
}

void Chat::signUp() 
{
	std::string login, password, name;
	std::cout << "Login: ";
	std::cin >> login;
	std::cout << "Password: ";
	std::cin >> password;
	std::cout << "Name : ";
	std::cin >> name;

	if (getUserByLogin(login) || login == "all")
	{
		throw UserLoginExp();
	}

	if (getUserByName(name) || name == "all")
	{
		throw UserNameExp();
	}
	User user = User(login, password, name);
	users_.push_back(user);
	currentUser_ = std::make_shared<User>(user);
}

void Chat::showChat() const // show mess
{
	std::string from;
	std::string to;
	std::cout << "Chat" << std::endl;

	for (auto& mess : messages_)
	{
		// show mess
		if (currentUser_->getUserLogin() == mess.getFrom() || currentUser_->getUserLogin() == mess.getTo() || mess.getTo() == "all")
		{
			from = (currentUser_->getUserLogin() == mess.getFrom()) ? "ß" : getUserByLogin(mess.getFrom())->getUserName();

			if (mess.getTo() == "all")
			{
				to = "(All)";
			}
			else
			{
				to = (currentUser_->getUserLogin() == mess.getTo()) ? "ß" : getUserByLogin(mess.getTo())->getUserName();
			}
			std::cout << "From : " << from << " To : " << to << std::endl;
			std::cout << "Text : " << mess.getText() << std::endl;
		}
	}
	std::cout << "-------------------" << std::endl;
}

void Chat::showLoginMenu() // start menu
{
	currentUser_ = nullptr;
	char operation;

	do
	{
		std::cout << "----------" << std::endl;
		std::cout << "Login  - 1" << std::endl;
		std::cout << "SignUp - 2" << std::endl;
		std::cout << "Exit   - 0" << std::endl;
		std::cout << "----------" << std::endl;
		
		std::cin >> operation;

		switch (operation)
		{
		case '1':
			login();
			break;
		case '2':
			try
			{
				signUp();
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
			break;

		case '0':
			isChatWork_ = false;
			break;
		default:
			std::cout << "1 or 2 " << std::endl;
			break;

		}
	} while (!currentUser_ && isChatWork_);
}

void Chat::showUserMenu() // Menu
{
	char operation;

	std::cout << "Hi, " << currentUser_->getUserName() << std::endl;

	while (currentUser_)
	{
		std::cout << "(1)Show Chat | (2)Send message | (3)Users | (0)Logout ";

		std::cout << std::endl;	

		std::cin >> operation;
		switch (operation)
		{
		case '1':
			showChat();
			break;
		case '2':
			addMessage();
			break;
		case '3':
			showAllUsersName();
			break;
		case '0':
			currentUser_ = nullptr;
			break;
		default:
			std::cout << "Unknown choice " << std::endl;
			break;
		}
	}
}

void Chat::showAllUsersName() const //show user names
{
	std::cout << " Chat " << std::endl;
	for (auto& user : users_)
	{
		std::cout << user.getUserName();
		if (currentUser_->getUserLogin() == user.getUserLogin())
			std::cout << "Me";
		std::cout << std::endl;
	}
	std::cout << "---------------" << std::endl;
}

void Chat::addMessage() // mess
{
	std::string to, text;
	std::cout << "To (name or all): ";
	std::cin >> to;
	std::cout << "Text: ";
	std::cin.ignore(); 
	getline(std::cin, text); 

	if (!(to == "all" || getUserByName(to))) // if do not find name
	{
		std::cout << "Can not find  " << to << std::endl;
		return;
	}
	if (to == "all")
		messages_.push_back(Message{ currentUser_->getUserLogin(), "all", text });
	else
		messages_.push_back(Message{ currentUser_->getUserLogin(), getUserByName(to)->getUserLogin(), text });
}
Chat::Chat(const char* serverIp, int serverPort, int bufferLength)
	: serverIp(serverIp), serverPort(serverPort), bufferLength(bufferLength)
{
	// ini socket
	initSocket();

	// address socket
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = inet_addr(serverIp);
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);
	serverAddr.sin_port = htons(serverPort);
	
	memset(&clientAddr, 0, sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddr.sin_port = htons(0);

	// length address
	clientAddrLen = sizeof(clientAddr);

	// Memory for mess
	receiveBuffer = new char[bufferLength];
	sendBuffer = new char[bufferLength];
}
Chat::~Chat()
{
}
void Chat::initSocket()
{
	// ini Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		throw std::runtime_error("Failed to initialize Winsock");
	}

	// create UDP-socket
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET)
	{
		WSACleanup();
		throw std::runtime_error("Failed to create UDP socket");
	}

	// socket + address
	if (bind(udpSocket, (SOCKADDR*)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR)
	{
		closesocket(udpSocket);
		WSACleanup();
		throw std::runtime_error("Failed to bind UDP socket");
	}
}
void Chat::sendMessage(const char* message)
{
	// send mess to server
	int result = sendto(udpSocket, message, strlen(message) + 1, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		closesocket(udpSocket);
		WSACleanup();
		throw std::runtime_error("Failed to send message");
	}
}
void Chat::receiveMessage()
{
	// mess from server
	int result = recvfrom(udpSocket, receiveBuffer, bufferLength, 0, (SOCKADDR*)&clientAddr, &clientAddrLen);
	if (result == SOCKET_ERROR)
	{
		closesocket(udpSocket);
		WSACleanup();
		throw std::runtime_error("Error receiving data from server");
	}

	// show on screen mess
	std::cout << "Received message from server: " << receiveBuffer << std::endl;
}