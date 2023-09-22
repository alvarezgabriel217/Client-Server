#pragma once
#include <fstream>
#include <string>

class Server
{
	//SOCKETS
	SOCKET listenSocket;
	SOCKET ComSocket;
	SOCKET BroadcastSocket;

	//GLOBAL VARIABLES
	timeval timeout = { 1, 0 };
	fd_set masterSet;
	fd_set readySet;
	time_t now;


public:

	/*std::string currentTime()
	{
		now = time(0);
		std::string returnedTime = asctime(localtime(&now));
		return returnedTime;
	}*/

	int Init(uint16_t port)
	{
		BOOL optVal = TRUE;
		int optLen = sizeof(BOOL);

		IO::fp = fopen("Log.txt", "a");


		//SOCKET CREATION
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		BroadcastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (listenSocket == INVALID_SOCKET)
		{
			std::cout << "Error Creating Socket\n";

			fprintf(IO::fp, "%s", "Socket was unsuccessfully Created");

			return 5;
		}
		if (BroadcastSocket == INVALID_SOCKET)
		{
			std::cout << "Error Creating Broadcast Socket\n";

			fprintf(IO::fp, "%s", "Broadcast Socket was unsuccessfully Created");

			return 5;
		}

		fprintf(IO::fp, "%s", "Socket Created\n");
		fprintf(IO::fp, "%s", "Broadcast Socket Created\n");

		int broadcast = setsockopt(BroadcastSocket, SOL_SOCKET, SO_BROADCAST, (char*)&optVal, optLen);
		if (broadcast == SOCKET_ERROR)
		{
			std::cout << "Error changing socket options\n";

			return 3;
		}
		int reuse = setsockopt(BroadcastSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, optLen);

		//BIND
		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
		serverAddr.sin_port = htons(port);

		int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (result == SOCKET_ERROR)
		{
			std::cout << "Error Binding\n";

			fprintf(IO::fp, "%s", "Failed to bind Socket");

			return 3;
		}

		fprintf(IO::fp, "%s", "Binding Successful\n");

		sockaddr_in broadcastAddr2;
		broadcastAddr2.sin_family = AF_INET;
		broadcastAddr2.sin_addr.S_un.S_addr = INADDR_ANY;
		broadcastAddr2.sin_port = htons(28500);

		sockaddr_in broadcastAddr;
		broadcastAddr.sin_family = AF_INET;
		broadcastAddr.sin_addr.S_un.S_addr = INADDR_BROADCAST;
		broadcastAddr.sin_port = htons(31338);

		int BroadcastResult = bind(BroadcastSocket, (SOCKADDR*)&broadcastAddr2, sizeof(broadcastAddr2));
		if (BroadcastResult == SOCKET_ERROR)
		{
			std::cout << "Error Binding\n";

			fprintf(IO::fp, "%s", "Failed to bind Broadcast Socket\n");

			return 3;
		}

		fprintf(IO::fp, "%s", "Binding Successful\n");


		//LISTEN
		result = listen(listenSocket, 5);
		if (result == SOCKET_ERROR)
		{
			std::cout << "Error Lisening\n";

			return 5;
		}
		std::cout << "Waiting for connection...\n";

		fprintf(IO::fp, "%s", "Waiting for connection...\n");



		FD_ZERO(&masterSet);
		FD_SET(listenSocket, &masterSet);
		//MULTIPLEXING
		while (true)
		{
			FD_ZERO(&readySet);
			readySet = masterSet;

			result = select(0, &readySet, NULL, NULL, &timeout);

			//BROADCASTING
			IO::writeBuffer2 = "127.0.0.55/" + std::to_string(port);
			result = sendto(BroadcastSocket, &IO::writeBuffer2[0], IO::writeBuffer2.size()+1, 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
			if (result == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				std::cout << error;
				std::cout << "Error Broadcasting Message\n";
				fprintf(IO::fp, "%s", "Error Broadcasting Message\n");
				return 1;
			}
			std::cout << "IP: 127.0.0.55 and Port: " << std::to_string(port) << "\n";
			fprintf(IO::fp, "%s" "%s" "%s", "IP: 127.0.0.55 and Port: ", std::to_string(port).c_str(), "\n");

			if (FD_ISSET(listenSocket, &readySet))
			{
				int serverAddrSize = sizeof(serverAddr);
				ComSocket = accept(listenSocket, (SOCKADDR*)&serverAddr, &serverAddrSize);
				if (ComSocket == INVALID_SOCKET)
				{
					std::cout << "Error Accepting\n";
					fprintf(IO::fp, "%s", "Error Accepting client");

					int error = WSAGetLastError();
					if (error == WSAESHUTDOWN)
					{
						return 1;
					}
					else
					{
						return 4;
					}
				}

				FD_SET(ComSocket, &masterSet);
				std::cout << "New Client Connected\n";

				fprintf(IO::fp, "%s", "New Client Connected\n");
			}

			for (int i = 0; i < masterSet.fd_count; i++)
			{
				if (readySet.fd_array[i] != listenSocket && FD_ISSET(masterSet.fd_array[i], &readySet))
				{
					std::cout << "Reading from clients...\n";

					if (IO::readMessage(readySet.fd_array[i], IO::readBuffer, 256) != 0)
					{
						std::cout << IO::users[i - 1].username << " has forcefully disconnected.\n";
						fprintf(IO::fp, "%s" "%s", IO::users[i - 1].username.c_str(), " Has forcefully disconnected\n");
						IO::users.erase(std::next(IO::users.begin(), i - 1));
						shutdown(readySet.fd_array[i], SD_BOTH);
						closesocket(readySet.fd_array[i]);
						FD_CLR(readySet.fd_array[i], &masterSet);
						FD_CLR(readySet.fd_array[i], &readySet);
						i = 0;
					}
					else
					{
						std::string readString = IO::readBuffer;
						if (readString.find("$register") != std::string::npos)
						{
							if (IO::Register() == SV_FULL)
							{
								IO::writeBuffer = "SV_FULL";
								IO::sendMessage(masterSet.fd_array[i], &IO::writeBuffer[0], IO::writeBuffer.size() + 1);

								std::cout << " No space left in the chat room" << std::endl;
								fprintf(IO::fp, "%s", " No space left in the chat room\n");

								shutdown(readySet.fd_array[i], SD_BOTH);
								closesocket(readySet.fd_array[i]);
								FD_CLR(readySet.fd_array[i], &masterSet);
								FD_CLR(readySet.fd_array[i], &readySet);
								i = 0;
							}
							else if (IO::Register() == SV_SUCCESS)
							{
								std::string username = readString.substr(readString.find("<") + 1, (readString.find(">") - readString.find("<") - 1));
								IO::users.push_back({ readySet.fd_array[i], username });

								std::cout << username << " has joined the chat" << std::endl;
								fprintf(IO::fp, "%s" "%s", username.c_str(), " has joined the chat\n");

								IO::writeBuffer = "SV_SUCCESS";
								IO::sendMessage(masterSet.fd_array[i], &IO::writeBuffer[0], IO::writeBuffer.size() + 1);
							}
						}
						else if (readString.find("$getlist") != std::string::npos)
						{
							fprintf(IO::fp, "%s" "%s" "%s" "%s", IO::users[i - 1].username.c_str(), ": ", IO::readBuffer, "\n");
							if (IO::users.size() > 0)
							{
								std::cout << "Sending user list to " << IO::users[i - 1].username << "...\n";
								fprintf(IO::fp, "%s" "%s", IO::users[i - 1].username.c_str(), " has requested the user list\n");
								IO::writeBuffer += "Users: [ ";
								for (int x = 0; x < IO::users.size(); x++)
								{
									IO::writeBuffer += IO::users[x].username;
									if (x != IO::users.size() - 1)
									{
										IO::writeBuffer += ", ";
									}
								}
								IO::writeBuffer += " ]";
								fprintf(IO::fp, "%s" "%s", IO::writeBuffer.c_str(), "\n");
							}
							else
							{
								IO::writeBuffer = "User List is empty, please try again later.";
								fprintf(IO::fp, "%s", "User List was requested but there were no users connected.\n");
							}
							IO::sendMessage(masterSet.fd_array[i], &IO::writeBuffer[0], IO::writeBuffer.size() + 1);
						}
						else if (readString.find("$exit") != std::string::npos)
						{
							fprintf(IO::fp, "%s" "%s" "%s" "%s", IO::users[i - 1].username.c_str(), ": ", IO::readBuffer, "\n");
							std::cout << IO::users[i - 1].username.c_str() << " has left the chat\n";
							fprintf(IO::fp, "%s" "%s", IO::users[i - 1].username.c_str(), " has left the chat\n");
							IO::users.erase(std::next(IO::users.begin(), i - 1));
							shutdown(readySet.fd_array[i], SD_BOTH);
							closesocket(readySet.fd_array[i]);
							FD_CLR(readySet.fd_array[i], &masterSet);
							FD_CLR(readySet.fd_array[i], &readySet);
							i = 0;
						}
						else if ((readString.find("$getlog") != std::string::npos))
						{

							fprintf(IO::fp, "%s" "%s" "%s" "%s", IO::users[i - 1].username.c_str(), ": ", IO::readBuffer, "\n");
							fprintf(IO::fp, "%s" "%s", IO::users[i - 1].username.c_str(), " has requested the log chat\n");
							std::cout << "Sending log...\n";

							fclose(IO::fp);
							std::ifstream myFile("Log.txt");
							IO::writeBuffer2 = "<Log>\n";
							char myChar;
							if (myFile.is_open())
							{
								while (myFile)
								{
									myChar = myFile.get();
									IO::writeBuffer2 += myChar;
								}
							}
							else
							{
								std::cout << "Failed at opening file";
							}

							IO::sendMessage(masterSet.fd_array[i], &IO::writeBuffer2[0], IO::writeBuffer2.size() + 1);
							IO::fp = fopen("Log.txt", "a");
							IO::writeBuffer2 = "\0";
						}
						else
						{
							std::cout << IO::users[i - 1].username << ": " << IO::readBuffer << std::endl;
							fprintf(IO::fp, "%s" "%s" "%s" "%s", IO::users[i - 1].username.c_str(), ": ", IO::readBuffer, "\n");
							IO::sendMessage(masterSet.fd_array[i], &IO::writeBuffer[0], IO::writeBuffer.size() + 1);
						}
						strcpy(IO::readBuffer, "\0");
						IO::writeBuffer = "\0";
					}
				}
			}

		}
		return 0;
	}


};
