#ifndef _RSC_SOCKET_H
#define _RSC_SOCKET_H 1

#include "rsc.h"

/* 在客户端创建一个socket连接，返回socket文件描述符 */
int client_connect_socket(char* ipaddr, int port);

#endif