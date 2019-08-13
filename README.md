# A C++ High Performance Chat Server and Client

## Introduction
本项目为C++11编写的多线程聊天服务器和客户端，客户端实现了自动编解码和自动重连，服务器实现了自动转发，并实现了异步日志，记录服务器和客户端的运行状态。

## Envoirment
* OS：Linux 18.04 	
* Complier： g++ 7.4

## Technical points
* 使用Epoll水平触发的IO多路复用技术，非阻塞IO，使用Reactor模式
* 使用多线程充分利用多核CPU，服务器创线程池避免线程频繁创建和销毁的开销，客户端采用双线程，分别负责读写标准IO和sockfd，实现同步读写
* 主线程只负责accept请求，并以Round Robin的方式分发给其他IO线程
* 利用LengthHead对消息进行编解码，服务器自动分发消息
* 为避免内存泄漏，使用智能指针和RAII机制
* 为减少锁争用，使用copy-on-write技术
* 实现了线程的异步唤醒
* 客户端实现了自动重连
* 实现了高性能异步日志，采用双缓冲技术，前台线程写缓冲，后台线程写磁盘

## MODEL
并发模型为Reactor+非阻塞IO+线程池，主线程负责accept新连接，并以Round Robin的方式分发连接
