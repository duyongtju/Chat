#include"ChatClient.h"
#include"../base/AsyncLogging.h"

#include<stdio.h>
#include<iostream>

off_t kRollSize = 500 * 1000 * 1000;

muduo::AsyncLogging* g_asyncLog = NULL;

void asyncOutput(const char* msg, int len)
{
	g_asyncLog->append(msg, len);
}

int main(int argc, char *argv[])
{
	muduo::AsyncLogging log("ChatClient",kRollSize);
	g_asyncLog = &log;
	muduo::Logger::setOutput(asyncOutput);
	log.start();

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