#ifndef _RSC_CLIENT_H
#define _RSC_CLIENT_H  1

// 远程系统调用请求编组
char * syscall_request_encode(struct rsc_header *header, struct user_regs_struct *user_regs);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  

/* 远程系统调用请求编组辅助函数，用于不同系统调用分类的指针参数处理 */
char * pointer_encode_client(struct rsc_header * header);
char * out_pointer_encode_client(unsigned int p_location, unsigned int p_count, struct rsc_header * header);
char * in_pointer_encode_client(unsigned int p_location, unsigned int p_count, unsigned long long int addr, struct rsc_header * header);
char * io_pointer_encode_client(struct io_rsc_pointer io_pointer, unsigned int * buffer_size);
 
// 远程系统调用请求返回结果解组
char * syscall_return_decode(char * syscall_return);

// 在客户端创建一个socket连接，返回socket文件描述符
int client_connect_socket(char* ipaddr, int port);

#endif