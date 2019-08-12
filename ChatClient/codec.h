#pragma once

#include"../base/Logging.h"
#include"../net/Buffer.h"
#include"../net/Endian.h"
#include"../net/TcpConnection.h"

class LengthHeaderCodec :muduo::noncopyable
{
public:
	typedef std::function<void(const muduo::net::TcpConnectionPtr&,
		const muduo::string&, muduo::Timestamp)> StringMessageCallback;
	LengthHeaderCodec()
	{
	}

	void onMessage(const muduo::net::TcpConnectionPtr& conn,
		muduo::net::Buffer* buf, muduo::Timestamp receiveTime)
	{
		while (buf->readableBytes() >= kHeaderLen)
		{
			const void* data = buf->peek();
			int32_t be32 = *static_cast<const int32_t*>(data);
			const int32_t len = muduo::net::sockets::networkToHost32(be32);
			if (len > 65536 || len < 0)
			{
				LOG_ERROR << "Invalid length " << len;
				conn->shutdown();
				break;
			}
			else if (buf->readableBytes() >= len + kHeaderLen)
			{
				buf->retrieve(kHeaderLen);
				std::string message(buf->peek(), len);
				printf("<<< %s\n", message.data());
				buf->retrieve(len);
			}
			else 
			{
				break;
			}
		}
	}

	void send(muduo::net::TcpConnectionPtr conn,
		const muduo::string& message)
	{
		muduo::net::Buffer buf;
		buf.append(message.data(), message.size());
		int32_t len = static_cast<int32_t>(message.size());
		int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
		buf.prepend(&be32, sizeof be32);
		conn->send(&buf);
	}

private:
	const static size_t kHeaderLen = sizeof(int32_t);
};