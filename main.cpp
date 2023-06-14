#include <iostream>
#include "Chat.h"

int main()
{
	setlocale(LC_ALL, "");  
	std::cout << "Welcome to Chat." << std::endl;
	
	std::cout << "----------------" << std::endl;

	Chat chat("127.0.0.1", 8080, 1024); 

	chat.start(); 

	while (chat.isChatWork()) {  

		chat.showLoginMenu(); 

		while (chat.getCurrentUser()) {
			chat.showUserMenu(); 
		}
	}
	return 0;
}