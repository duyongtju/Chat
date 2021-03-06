# A C++ High Performance Chat Server and Client

## Introduction

本项目为C++11编写的多线程聊天服务器和客户端，客户端实现了自动编解码和自动重连，服务器实现了自动转发，并实现了异步日志，记录服务器和客户端的运行状态。

## Envoirment

* OS：Linux 18.04 	
* Complier： g++ 7.4

## Technical points

* 基于Reactor模式，使用IO多路复用+非阻塞IO。
* 使用多线程充分利用多核CPU，服务器创线程池避免线程频繁创建和销毁的开销，客户端采用双线程，分别负责读写标准IO和sockfd，实现同步读写
* 主线程只负责accept请求，并以Round Robin的方式分发给其他IO线程
* 利用LengthHead对消息进行编解码，服务器自动分发消息
* 为避免内存泄漏，使用智能指针和RAII机制
* 为减少锁争用，使用copy-on-write技术
* 实现了线程的异步唤醒
* 客户端实现了自动重连
* 实现了定时器功能
* 实现了高性能异步日志，采用双缓冲技术，前台线程写缓冲，后台线程写磁盘

## Model

并发模型为Reactor+非阻塞IO+线程池，主线程负责accept新连接，并以Round Robin的方式分发连接

![并发模型](https://github.com/duyongtju/Chat/blob/master/datum/model.png)



## 连接维护

* 建立连接
* 限制最大并发连接数
* 断开连接

详情见 [连接维护](https://github.com/duyongtju/Chat/blob/master/Connection.md)

## 代码统计

![cloc](https://github.com/duyongtju/Chat/blob/master/datum/cloc.png)

## 工作详情

* 服务器
![服务器](https://github.com/duyongtju/Chat/blob/master/datum/Server.png)
* 客户端1
![客户端1](https://github.com/duyongtju/Chat/blob/master/datum/Client1.png)
* 客户端2
![客户端2](https://github.com/duyongtju/Chat/blob/master/datum/Client2.png)
* 自动重连
![自动重连](https://github.com/duyongtju/Chat/blob/master/datum/Reconnection.png)


