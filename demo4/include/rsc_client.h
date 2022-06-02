#ifndef RSC_CLIENT_H_
#define RSC_CLIENT_H_  1

// remote syscall request encode and remote syscall result decode
char *  RequestEncode(struct user_regs_struct * regs, struct rsc_header * header);                                                                         
int     ResultDecode(struct user_regs_struct * regs, struct rsc_header * header, char * extra_buffer);

// pointer paraments handle
char *  PointerEncode(struct rsc_header * header);
char *  OutputPointerEncode(unsigned int p_location, unsigned int p_count, struct rsc_header * header);
char *  InputPointerEncode(unsigned int p_location, unsigned int p_count, unsigned long long addr_in, struct rsc_header * header);
int     OutputPointerDecode(struct user_regs_struct * regs, struct rsc_header * header, char * extra_buffer);

// create socket connect in client
int     SocketConnect(char* ip_addr, int port);

// syscall bitmap operation
void    ResetBitmap(unsigned long long int * syscall_bitmap, unsigned int syscall);
int     IsSet(unsigned long long int * syscall_bitmap, unsigned int syscall);
void    SetBitmap(unsigned long long int * syscall_bitmap, unsigned int syscall);


#endif