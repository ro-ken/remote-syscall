# SystemCall-Remote About

[toc]

## 赛题详细

### 项目描述与背景

系统调用(Syscall)作为操作系统对用户态提供的编程接口，通常是用户态进程和内核之间最底层的通信接口。这里我们只考虑为Unix Like的系统平台做相关设计，在Unix Like环境上OS为每个特定的系统调用提供了系统调用编号，通过栈和寄存器来传递系统调用的参数，通常系统调用服务的个数是有限的，并且libc为我们封装好了更加友好的系统调用接口。这里以进程从文件中读取内容为例：

```c
+--------+               +-------+
| Reader |               | Linux |
+--------+               +-------+
          |                  |
          |      open        |
          | ---------------> |                 +---------+
          |                  |                 |         |
          |      read        |                 |         |
          | ---------------> | --------------> |  Disk   |
          |                  |                 |         |
          |      close       |                 |         |
          | ---------------> |                 +---------+
          |                  |
          |                  |
```

本项目的目标是实现一个Remote Syscall执行框架，通过这个执行框架用户可以**将客户端的一个指定进程**的系统调用进行截获，并转发到服务端上下文进行执行，然后将系统调用结果返回给客户端。项目的本质是为了实现一个远程系统调用模拟环境。

### 功能描述

#### 基础功能：

* 1、支持远程系统的调用的基本功能，能够将**进程内特定的系统调用**拉远到服务端执行并返回对应的结果。

```c
+----------------+      +----------------+      +----------------+     +------------------+
| Reader process |      | Syscall Client |      | Syscall server |     | Operating System |
+-------+--------+      +----------------+      +----------------+     +------------------+
        |                       |                       |                      |
        |       stub_open       |                       |                      |
        | ------------------->  |     syscall_request   |                      |
        |                       | --------------------->|         open         |
        |                       |                       | -------------------->|
        |       stub read       |                       |                      |                +----------+
        | --------------------> |                       |                      |                |          |
        |                       |     syscall_reqeust   |                      |                |          |
        |                       | --------------------> |         read         |                |   disk   |
        |       stub close      |                       | -------------------> | -------------> |          |
        | --------------------> |                       |                      |                |          |
        |                       |     syscall_reqeust   |                      |                |          |
        |                       | --------------------> |                      |                |          |
        |                       |                       |         close        |                +----------+
        |                       |                       | -------------------> |
        |                       |                       |                      |
```

* 2、支持将**某个特定的本地进程的所有系统调用**拉远到服务端执行，使得本地进程能够执行在远程服务端上下文并返回正确的执行结果。

* 3、支持通过tcp或者virtio-vsock**两种不同通信通道来传递系统调用**，通信通道做到独立解耦。

#### 扩展功能：

* 1、支持对应用程序代码系统调用修改的方式，将应用程序的系统调用拉远到服务端上下文执行。
* 2、支持通过LD\_PRELOAD，或者ptrace和ebpf规则的方式将进程内特定的系统调用，做到非侵入式系统调用拉远。
* 3、尽量简化服务端执行上下文的架构。

#### 项目导师

方应 fangying.tommy@bytedance.com

#### 难度

高等

### 预期目标

完成基础功能和扩展功能的开发，并输出说明文档一篇。

## 可能会用到的技术

## 需要解决的难题

### 如何在本地进程中拦截系统调用并使系统调用在服务端执行而不是本地端

已知的一些前提：

1. 系统调用启动之后没办法去中断它
2. 进程使用系统调用过程是：进程调用库函数--》库函数调用系统调用



所以我们现在面临一些问题和抉择：

1. 本地系统调用不可终止：目前使用的拦截系统调用的工具是ptrace，而ptrace是在系统调用发生之前，glibc库函数调用系统调用之后拦截到系统调用的，这个时候系统调用已经启动了，并且是在本地启动的，而在系统调用启动之后是没办法去终止它的（一篇博客上看到的，真实性有待验证），哪怕服务器端将拦截到系统调用执行完的结果发送给本地进程，本地进程依旧还是要执行系统调用的。

#### method-1: 

1. 在本地端添加一个系统调用DoNothing，系统调用号10000（保证所有平台上都使用不到该系统调用号），该系统调用实际上什么也不做。
2. 启动RSCF（Remote System Call Frame）我们的远程系统调用框架）和待处理的进程D_Process
3. 使用ptrace拦截D_Process的所有系统调用，复制D_Processs的寄存器上下文，重构为RSC（Remote System Call）请求发送给远程服务端RSR
4. 远程服务端RSR执行完RSC之后，将执行结果发送回RSCF的client端，client端接收RSC回复，将执行结果写入到进程的寄存器中，并且修改系统调用号ORIG_EAX为10000（DoNothing），client继续D_Process的执行。
5. 因为D_Process的系统调用被重定向到DoNothing，该系统调用什么也不做，根据系统正常流程返回，但实际上D_Process想要执行的系统调用的结果已经写入到D_Process的上下文中了。

#### 通过ptrace

### 怎么在服务器端维护一个本地进程的上下文



## reference

### system call remote

#### 参考资料

1. Syscall Proxying： https://markowsky.us/papers/exploits/SyscallProxying.pdf

### strace

### ptrace

1. [使用 Ptrace 拦截和模拟 Linux 系统调用](https://www.anquanke.com/post/id/149409)
2. [使用 Ptrace 去拦截和仿真 Linux 系统调用](https://zhuanlan.zhihu.com/p/42898266)

### google gvisor

1. [gVisor：谷歌发布的一个用于提供安全隔离的轻量级容器运行时沙箱](https://www.infoq.cn/article/2018/05/gvisor-container-sandbox)
1. [Kubernetes的Kata Containers 与 gVisor - ghostwritten](https://blog.kelu.org/tech/2022/02/15/kubernetes-kata-gvisor.html)
1. [**gVisor是什么？可以解决什么问题**](https://blog.51cto.com/u_15127630/2770676)

### LD_PRELOAD

1. [LD_PRELOAD的偷梁换柱之能](https://www.cnblogs.com/net66/p/5609026.html)

### ebpf

1. [从Falco看如何利用eBPF检测系统调用](https://www.wangan.com/p/7fy7f4de44048f47)
2. [BPF 学习路径总结](https://www.modb.pro/db/82230)
3. https://github.com/DavadDi/bpf_study

