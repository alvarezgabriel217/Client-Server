#define _CRT_SECURE_NO_WARNINGS                 // turns off deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns off deprecated warnings for winsock

#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include "SharedInfo.h"
#include "Server.h"
#include "Client.h"
#pragma comment(lib,"Ws2_32.lib")

bool InputValidation(std::string string);

int main()
{
	WSADATA wsadata;
	WSAStartup(WINSOCK_VERSION, &wsadata);

	//USER INPUT
	uint16_t port;
	std::string tempPort;
	char address[256];
	int choice = 0;

	std::cout << "Would you like to make a Server or a Client?\n" << "1) Server\n2) Client\n";

	std::cin >> choice;
	std::cin.ignore();
	while (choice != 1 && choice != 2)
	{
		std::cout << "Invalid Option, please try again. ";
		std::cin >> choice;
		std::cin.ignore();
	}

	if (choice == 1) //SERVER CREATION
	{
		Server server;
		std::cout << "Enter a port number: ";
		std::cin >> tempPort;
		std::cin.ignore();

		while (true)
		{
			if (InputValidation(tempPort))
			{
				port = atoi(tempPort.c_str());
				if (port > 1024 && port < 65535)
				{
					break;
				}
				else
				{
					std::cout << "Invalid port number, please try again. ";
					std::cin >> tempPort;
					std::cin.ignore();
				}
			}
			else
			{
				std::cout << "Invalid port number, please try again. ";
				std::cin >> tempPort;
				std::cin.ignore();
			}
		}

		std::cout << "Enter a max number of clients: ";

		std::cin >> tempPort;
		std::cin.ignore();

		while (true)
		{
			if (InputValidation(tempPort))
			{
				IO::ChatCapacity = atoi(tempPort.c_str());
				if (IO::ChatCapacity > 0 && IO::ChatCapacity < 10)
				{
					break;
				}
				else
				{
					std::cout << "Invalid number, please try again. ";
					std::cin >> tempPort;
					std::cin.ignore();
				}
			}
			else
			{
				std::cout << "Invalid number, please try again. ";
				std::cin >> tempPort;
				std::cin.ignore();
			}
		}

		IO::fp = fopen("Log.txt", "w");
		server.Init(port);
		fclose(IO::fp);
	}
	else if (choice == 2) //CLIENT CREATION
	{
		Client client;
		/*std::cout << "Enter a port number: ";
		std::cin >> tempPort;
		std::cin.ignore();

		while (true)
		{
			if (InputValidation(tempPort))
			{
				port = atoi(tempPort.c_str());
				if (port > 1024 && port < 65535)
				{
					break;
				}
				else
				{
					std::cout << "Invalid port number, please try again. ";
					std::cin >> tempPort;
					std::cin.ignore();
				}
			}
			else
			{
				std::cout << "Invalid port number, please try again. ";
				std::cin >> tempPort;
				std::cin.ignore();
			}
		}

		std::cout << "Enter an address: ";
		std::cin >> address;
		std::cin.ignore();*/

		//client.Init(port, address);
		client.Init();

		////DEBUGGING
		//char address2[256];
		//strcpy(address2, "127.0.0.1");
		//client.Init(31337, address2);

		while (client.clientState)
		{
			client.Sending();
		}
	}

}

bool InputValidation(std::string string)
{
	if (string.size() <= 0)
	{
		return false;
	}
	for (int i = 0; i < string.size(); i++)
	{
		if (!isdigit(string[i]))
		{
			return false;
		}
	}
	return true;
}
