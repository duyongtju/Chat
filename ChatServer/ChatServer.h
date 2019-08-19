#pragma once

#include"../net/codec.h"

#include"../base/Logging.h"
#include"../base/Mutex.h"
#include"../base/Types.h"
#include"../net/EventLoop.h"
#include"../net/EventLoopThreadPool.h"
#include"../net/TcpConnection.h"
#include"../net/Acceptor.h"
#include"../net/SocketsOps.h"
#include"../base/Atomic.h"

#include<set>
#include<map>
#include<stdio.h>
#include<unistd.h>

using namespace muduo;
using namespace muduo::net;

typedef std::set<TcpConnectionPtr> ConnectionList;
typedef std::shared_ptr<ConnectionList> ConnectionListPtr;
typedef std::map<string, TcpConnectionPtr> ConnectionMap;

class ChatServer :noncopyable
{
public:
	ChatServer(EventLoop* loop, const InetAddress& listenAddr) :
		loop_(loop),
		ipPort_(listenAddr.toIpPort()),
		name_("ChatServer"),
		acceptor_(new Acceptor(loop, listenAddr, true)),
		threadPool_(new EventLoopThreadPool(loop, name_)),
		nextConnId_(1),
		codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3)),
		connectionListPtr_(new ConnectionList)
	{
		acceptor_->setNewConnectionCallback(
			std::bind(&ChatServer::newConnection, this, _1, _2));
	}
		
	void start()
	{
		if (started_.getAndSet(1) == 0)
		{
			threadPool_->start(NULL);

			loop_->runInLoop(
				std::bind(&Acceptor::listen, get_pointer(acceptor_)));
		}
	}

	void setThreadNum(int numThreads)
	{
		threadPool_->setThreadNum(numThreads);
	}
	
	void newConnection(int sockfd, const InetAddress& peerAddr)
	{
		EventLoop* ioLoop = threadPool_->getNextLoop();
		char buf[64];
		snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
		++nextConnId_;
		string connName = name_ + buf;

		LOG_INFO << "ChatServer::newConnection new connection [" << connName
			<< "] from " << peerAddr.toIpPort();
		InetAddress localAddr(sockets::getLocalAddr(sockfd));
		TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop,
			connName, sockfd, localAddr, peerAddr);
		connections_[connName] = conn;
		conn->setConnectionCallback(
			std::bind(&ChatServer::onConnection, this, _1));
		conn->setMessageCallback(
			std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
		conn->setCloseCallback(
			std::bind(&ChatServer::removeConnection, this, _1));
		ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
	}


private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		MutexLockGuard lock(mutex_);
		if (!connectionListPtr_.unique())
		{
			connectionListPtr_.reset(new ConnectionList(*connectionListPtr_));
		}

		if (conn->connected())
		{
			connectionListPtr_->insert(conn);
		}
		else
		{
			connectionListPtr_->erase(conn);
		}

	}

	void onStringMessage(const TcpConnectionPtr&,
		const string& message, Timestamp)
	{
		ConnectionListPtr connectionListPtr = getConnectionList();
		for (ConnectionList::iterator it = connectionListPtr->begin();
		it != connectionListPtr->end(); it++)
		{
			codec_.send(get_pointer(*it), message);
		}
	}

	ConnectionListPtr getConnectionList()
	{
		MutexLockGuard lock(mutex_);
		return connectionListPtr_;
	}
	
	void removeConnection(const TcpConnectionPtr& conn)
	{
		loop_->runInLoop(std::bind(&ChatServer::removeConnectionInLoop, this, conn));
	}

	void removeConnectionInLoop(const TcpConnectionPtr& conn)
	{
		LOG_INFO << "ChatServer::removeConnectionInLoop - connection " 
			<< conn->name();
		connections_.erase(conn->name());
		EventLoop* ioLoop = conn->getLoop();
		ioLoop->queueInLoop(
			std::bind(&TcpConnection::connectDestroyed, conn));
	}

	EventLoop* loop_;
	const string ipPort_;
	const string name_;
	std::unique_ptr<Acceptor> acceptor_;
	std::shared_ptr<EventLoopThreadPool> threadPool_;
	int nextConnId_;
	LengthHeaderCodec codec_;
	AtomicInt32 started_;
	ConnectionListPtr connectionListPtr_;
	MutexLock mutex_;
	ConnectionMap connections_;
};
