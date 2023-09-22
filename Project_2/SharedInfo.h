#pragma once
#define SV_FULL 1
#define SV_SUCCESS 0

namespace IO
{
	//STRUCTS
	struct UserInfo
	{
		SOCKET sock;
		std::string username;
	};

	//GLOBAL VARIABLES
	std::vector<UserInfo> users;
	char readBuffer[5000];
	std::string writeBuffer;
	char readBuffer2[5000];
	std::string writeBuffer2;
	unsigned long value = 1;
	FILE* fp;
	FILE* fp2;
	int ChatCapacity = 0;

	//FUNCTIONS AND COMMANDS
	int tcp_recv_whole(SOCKET s, char* buf, int len)
	{
		int total = 0;

		do
		{
			int ret = recv(s, buf + total, len - total, 0);
			if (ret < 1)
				return ret;
			else
				total += ret;

		} while (total < len);

		return total;
	}

	int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
	{
		int result;
		int bytesSent = 0;

		while (bytesSent < length)
		{
			result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);

			if (result <= 0)
				return result;

			bytesSent += result;
		}

		return bytesSent;
	}

	int readMessage(SOCKET s, char* buffer, int32_t size)
	{
		uint8_t length = 0;
		int result = tcp_recv_whole(s, (char*)&length, 1);
		if (result == SOCKET_ERROR)
		{
			return 2;
		}
		if (result == 0)
		{
			return 1;
		}
		if (length > size)
		{
			return 8;
		}

		result = tcp_recv_whole(s, (char*)buffer, length);
		if (result == SOCKET_ERROR)
		{
			return 2;
		}
		if (result == 0)
		{
			return 1;
		}
		if (length > size)
		{
			return 8;
		}

		return 0;
	}
	int sendMessage(SOCKET s, char* data, int32_t length)
	{
		int result = tcp_send_whole(s, (char*)&length, 1);
		if (result == SOCKET_ERROR)
		{
			return 2;
		}
		if (result == 0)
		{
			return 1;
		}
		if (length > 5000 || length < 0)
		{
			return 8;
		}



		result = tcp_send_whole(s, data, length);
		if (result == SOCKET_ERROR)
		{
			return 2;
		}
		if (result == 0)
		{
			return 1;
		}
		if (length > 5000 || length < 0)
		{
			return 8;
		}
		return 0;
	}

	int Register()
	{
		if (users.size() >= ChatCapacity)
		{
			return SV_FULL;
		}
		else
		{
			//std::cout << "[DEBUGGING] SUCCESSFULLY REGISTERED" << std::endl;
			return SV_SUCCESS;
		}
	}

	/*FILE* fopen(const char *filename, const char *mode)
	{

	}*/


}