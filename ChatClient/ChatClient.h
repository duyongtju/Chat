#pragma once
#include"../base/Logging.h"
#include"../base/Mutex.h"
#include"../net/EventLoop.h"
#include"../net/EventLoopThread.h"
#include"../net/Connector.h"
#include"../net/TcpConnection.h"
#include"../net/SocketsOps.h"

#include<iostream>
#include<stdio.h>
#include<unistd.h>

using namespace muduo;
using namespace muduo::net;

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



class ChatClient :noncopyable
{
public:
	typedef std::shared_ptr<Connector> ConnectorPtr;
	 
	ChatClient(EventLoop* loop, InetAddress& serverAddr)
		:loop_(loop),
		connector_(new Connector(loop, serverAddr)),
		connect_(true),
		codec_()
	{
		connector_->setNewConnectionCallback(
			std::bind(&ChatClient::newConnection, this, _1));

		LOG_INFO << "ChatClient::ChatClient - connector "
			<< get_pointer(connector_);
	}

	~ChatClient()
	{
		LOG_INFO << "ChatClient::~ChatClient";
	}


	void connect()
	{
		LOG_INFO << "TcpClient connecting to "
			<< connector_->serverAddress().toIpPort();
		connect_ = true;
		connector_->start();
	}

	void newConnection(int sockfd)
	{
		InetAddress peerAddr(sockets::getPeerAddr(sockfd));
		char buf[32];
		snprintf(buf, sizeof buf, "ChatCLient:%s", peerAddr.toIpPort().c_str());
		InetAddress localAddr(sockets::getLocalAddr(sockfd));

		TcpConnectionPtr  conn(new TcpConnection(loop_, buf,
			sockfd, localAddr, peerAddr));
		connection_ = conn;
		connection_->setConnectionCallback(std::bind(&ChatClient::onConnection, this, _1));
		connection_->setMessageCallback(std::bind(&LengthHeaderCodec::onMessage, &codec_,
			_1, _2, _3));
		connection_->setCloseCallback(std::bind(&ChatClient::removeConnection, this, _1));
		connection_->connectEstablished();
	}

	void removeConnection(const TcpConnectionPtr& conn)
	{
		connection_.reset();
		loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
		if (connect_)
		{
			LOG_INFO << "ChatClient - Reconnecting to"
				<< connector_->serverAddress().toIpPort();
			connector_->restart();
		}
	}


	void write(const std::string& message)
	{
		if (connection_)
		{
			codec_.send(connection_, message);
		}
	}

	void disconnect()
	{
		connect_ = false;
		connection_->forceClose();
	}
private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		if (conn->connected())
		{
			connection_ = conn;
		}
		else
		{
			connection_.reset();
		}
	}
	
	EventLoop* loop_;
	ConnectorPtr connector_;
	bool connect_;
	LengthHeaderCodec codec_;
	TcpConnectionPtr connection_;
};

