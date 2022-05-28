#ifndef _RSC_CLIENT_H
#define _RSC_CLIENT_H  1

// RSCQ
char * syscall_request_encode(struct rsc_header *header, struct user_regs_struct *user_regs);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
int syscall_return_decode(struct rsc_header * header, char * syscall_result);

// pointer paraments handle
char * pointer_encode_client(struct rsc_header * header);
char * out_pointer_encode_client(unsigned int p_location, unsigned int p_count, struct rsc_header * header);
char * in_pointer_encode_client(unsigned int p_location, unsigned int p_count, unsigned long long int addr, struct rsc_header * header);
char * io_pointer_encode_client(struct io_rsc_pointer io_pointer, unsigned int * buffer_size);
int out_pointer_decode_client(struct rsc_header * header, struct user_regs_struct ru_regs);
// create socket connect in client
int socket_connect_client(char* ipaddr, int port);

#endif