#include"ChatServer.h"
#include"../base/AsyncLogging.h"

off_t kRollSize = 500 * 1000 * 1000;

muduo::AsyncLogging* g_asyncLog = NULL;

void asyncOutput(const char* msg, int len)
{
	g_asyncLog->append(msg, len);
}

int main(int argc, char *argv[])
{
	muduo::AsyncLogging log("ChatServer",kRollSize);
	g_asyncLog = &log;
	muduo::Logger::setOutput(asyncOutput);
	log.start();

	LOG_INFO << "pid = " << getpid();
	if (argc > 1)
	{
		EventLoop loop;
		uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
		InetAddress serverAddr(port);
		ChatServer server(&loop, serverAddr);
		if (argc > 2)
		{
			server.setThreadNum(atoi(argv[2]));
		}
		server.start();
		loop.loop();
	}
	else
	{
		printf("Usage: %s port [thread_num]\n", argv[0]);
	}
}