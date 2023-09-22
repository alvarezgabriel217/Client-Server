#pragma once

class Client
{
	SOCKET ComSocket;
	SOCKET BroadcastSocket;
	bool registered = false;
	bool msgSent = false;
	std::string username;

public:
	bool clientState = true;

	//int Init(uint16_t port, char* address)
	int Init()
	{
		BOOL optVal = TRUE;
		int optLen = sizeof(BOOL);

		//SOCKET CREATION
		ComSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ComSocket == INVALID_SOCKET)
		{
			return 5;
		}

		//BROADCASTING
		BroadcastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (BroadcastSocket == INVALID_SOCKET)
		{
			std::cout << "Error Creating Broadcast Socket\n";

			return 5;
		}

		int reuse = setsockopt(BroadcastSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, optLen);

		sockaddr_in broadcastAddr;
		int broadcastLen = sizeof(broadcastAddr);
		broadcastAddr.sin_family = AF_INET;
		broadcastAddr.sin_addr.S_un.S_addr = INADDR_ANY;
		broadcastAddr.sin_port = htons(31338);

		sockaddr_in broadcastAddr2;
		int broadcastLen2 = sizeof(broadcastAddr2);
		broadcastAddr2.sin_family = AF_INET;
		broadcastAddr2.sin_addr.S_un.S_addr = INADDR_ANY;
		broadcastAddr2.sin_port = htons(28500);

		int BroadcastResult = bind(BroadcastSocket, (SOCKADDR*)&broadcastAddr, sizeof(broadcastAddr));
		if (BroadcastResult == SOCKET_ERROR)
		{
			std::cout << "Error Binding\n";

			fprintf(IO::fp, "%s", "Failed to bind Broadcast Socket\n");

			return 3;
		}

		int result = recvfrom(BroadcastSocket, IO::readBuffer, 256, 0, (sockaddr*)&broadcastAddr2, &broadcastLen2);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			std::cout << error;
			std::cout << "Error receiving broadcast\n";
			fprintf(IO::fp, "%s", "Failed to receive from Broadcast Socket\n");

		}

		shutdown(BroadcastSocket, SD_BOTH);
		closesocket(BroadcastSocket);

		std::string newBuffer = std::string(IO::readBuffer);
		int newPort = atoi(newBuffer.substr(newBuffer.find("/")+1).c_str());
		std::string newAddress = newBuffer.substr(0, newBuffer.find("/"));

		std::cout << "IP:" << newAddress << " and Port: " << newPort << "\n";

		//CONNECT
		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.S_un.S_addr = inet_addr(newAddress.c_str());
		serverAddr.sin_port = htons(newPort);

		result = connect(ComSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

		int errorCode = WSAGetLastError();
		if (errorCode == WSAEWOULDBLOCK)
		{
			std::cout << "[DEBUGGING] WOULDBLOCK\n";
		}
		else
		{
			if (serverAddr.sin_addr.S_un.S_addr == INADDR_NONE)
			{
				return 7;
			}
			if (result == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error == WSAESHUTDOWN)
				{
					std::cout << "Failed to connect" << std::endl;
					return 1;
				}
				else
				{
					std::cout << "Failed to connect" << std::endl;
					clientState = false;
					return 4;
				}
			}
		}



		std::cout << "Connected to Server" << std::endl;

		std::cout << "Enter a username: \n";
		std::getline(std::cin, username);

		return 0;
	}

	void Sending()
	{
		if (!registered)
		{
			std::cout << "You have not registered, please type $register to register.\n";
			std::getline(std::cin, IO::writeBuffer);
			while (IO::writeBuffer.find("$register") == std::string::npos)
			{
				std::cout << IO::writeBuffer;
				std::cout << "You have not registered, please type $register to register.\n";
				std::getline(std::cin, IO::writeBuffer);
			}
			IO::writeBuffer += "<" + username + ">";
			IO::sendMessage(ComSocket, &IO::writeBuffer[0], IO::writeBuffer.size() + 1);
			registered = true;


			IO::readMessage(ComSocket, IO::readBuffer, 256);

			if (strcmp(IO::readBuffer, "SV_SUCCESS") == 0)
			{
				std::cout << "You were added successfully!\n";
				strcpy(IO::readBuffer, "\0");
				IO::writeBuffer = "\0";
				return;
			}
			else if (strcmp(IO::readBuffer, "SV_FULL") == 0)
			{
				std::cout << "Server is full, sorry for the inconvenience :(\n";

				std::cout << "Closing...";

				shutdown(ComSocket, SD_BOTH);
				closesocket(ComSocket);
				strcpy(IO::readBuffer, "\0");
				IO::writeBuffer = "\0";
				clientState = false;
				return;
			}
		}

		std::cout << "ENTER A MESSAGE: ";
		std::getline(std::cin, IO::writeBuffer);
		IO::sendMessage(ComSocket, &IO::writeBuffer[0], IO::writeBuffer.size() + 1);

		if (IO::writeBuffer.find("$exit") != std::string::npos)
		{
			shutdown(ComSocket, SD_BOTH);
			closesocket(ComSocket);
			clientState = false;
		}

		IO::readMessage(ComSocket, IO::readBuffer, 5000);


		std::string tempBuffer = IO::readBuffer;
		if (IO::writeBuffer.find("$getlog") != std::string::npos)
		{
			std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
			FILE* newFile = fopen((username + "Log.txt").c_str(), "w+");
			fprintf(newFile, "%s", tempBuffer.c_str());
			std::cout << tempBuffer;
			std::cout << username << "Log.txt has been generated\n";
			std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
			fclose(newFile);
		}
		else if(tempBuffer.find("Users") != std::string::npos)
		{
			std::cout << IO::readBuffer << std::endl;
		}

		strcpy(IO::readBuffer, "\0");
		IO::writeBuffer = "\0";
	}
};