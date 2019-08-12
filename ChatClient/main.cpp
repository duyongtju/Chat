#include"ChatClient.h"
#include<iostream>


int main(int argc, char *argv[])
{
	if (argc == 3)
	{
		EventLoopThread loopThread;
		uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
		InetAddress serverAddr(argv[1], port);

		ChatClient client(loopThread.startLoop(), serverAddr);
		client.connect();
		std::string line;
		while (std::getline(std::cin,line))
		{
			client.write(line);
		}
		client.disconnect();
		CurrentThread::sleepUsec(1000 * 1000);
	}
	else
	{
		printf("usage: %s host_ip port\n", argv[0]);
	}

}