# 连接维护


## 建立连接
* 连接是由Client发起的，通过socket(),bind(),connect()创建connecting sockfd（非阻塞），
Server被动建立连接，通过socket(),bind(),listen(),创建listenfd,并将listenfd注册到MainReactor中,使用epoll监听listenfd的读请求。
当TCP连接完成3次握手，会触发listenfd的读事件，调用accept()得到connectfd，并将该connectfd分发给SubReactor。

## 限制连接
服务器会限制并发连接数，当连接数达到服务器设置的最大连接数，服务器会主动关闭新到达的连接。

## 关闭连接

### 客户端
* 当标准IO输入输入 ctrl+D 时，会触发sockfd的close操作。
* 当网络错误或服务器死掉，客户端会关闭已连接connected sockfd，并重新创建connecting sockfd，不断尝试重新连接

### 服务器
* 正常情况下，服务器不会主动关闭连接，除非以下情况。
* 当EPOLLIN触发，但read()返回0的情况，会触发connected sockfd的close调用。


