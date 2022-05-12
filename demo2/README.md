# Remote System Call Demo2

* 实现了针对特定系统调用的远程系统调用推送，针对的系统调用是*getpid()*

## 项目结构

```c
+ rsc.h         // 头文件，包含RSC(Remote system call)的一些必要的库函数头文件、自定义数据结构、宏
+ rsc_socket.h  // 头文件，包含使用TCP进行通信的socket方法、数据结构的封装
+ rsc_client.c  // RSC的本地端框架
+ rsc_server.c  // RSC的服务端框架
+ Makefile
```

## 其他文件

```C
+ rsc_redict.c // 本地测试文件，使用父进程和子进程来模拟远程和本地的系统调用推送
```
