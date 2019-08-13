# 并发模型
程序使用Reactor模型，利用多线程提高并发度。为避免线程频繁创建和销毁的开销，在程序开始就创建一定数量线程的线程池。采用Epoll实现了IO复用。

## 线程
### 服务器中有3类线程
* 主线程：负责accept线连接，分发新连接给IO线程
* IO线程：负责读写connected sockfd
* LOG线程：负责向磁盘里写日志

### 客户端中也有3类线程
* 主线程：负责读写标准IO
* IO线程：负责网络连接和读写connected sockfd
* LOG线程： 负责向磁盘里写日志

## 并发模型
服务器的并发模型如下图所示

！[并发模型](https://github.com/duyongtju/Chat/blob/master/datum/model.png)

主线程中有一个MainReactor，负责accept新连接，并将新连接利用Round Robin算法分配给某个IO线程的SubReactor。
有多个IO线程，每个IO线程有一个SubReactor，并且维护一个独立的NIO Selector。
当主线程将新连接分配给某个SubReactor，该线程可能阻塞在epool_wait中，这是就要用到异步唤醒，本服务器利用eventfd实现异步唤醒，每个SubReactor中都会被注册一个eventfd，当新连接要分配给某个SubReactor时，就写eventfd，唤醒该线程。
