#include <sys/types.h>
#include <sys/socket.h>
#include<sys/syscall.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

 
#define PORT 9990
#define SIZE 1024
 
int creat_socket();
int wait_client(int listen_socket);
long  atol(const char *nptr);
int  atoi(const char *nptr);
char* itoa(int num,char* str,int radix);
void strtoargs(char* buf,int* number,long* RDI,long* RSI,long* RDX,long* R10,long* R8,long* R9);
void *hanld_client(void * args);
int do_syscall(int number,long RDI,long RSI,long RDX,long R10,long R8,long R9);

int main()
{
	int listen_socket = creat_socket();
	
	while(1)
	{
		int client_socket = wait_client(listen_socket);
		
		pthread_t id;
		pthread_create(&id, NULL, hanld_client, (void *)&client_socket);  //创建一个线程，来处理客户端。
		
		pthread_detach(id);   //把线程分离出去。
	}
	
	close(listen_socket);
	
	return 0;
}


void * hanld_client(void * args)    //信息处理函数
{
    int client_socket = *(int *)args;
	char buf[SIZE];
	while(1)
	{
		int ret = read(client_socket, buf, SIZE-1);
		if(ret == -1)
		{
			perror("read");
			break;
		}
		if(ret == 0)
		{
			break;
		}
		buf[ret] = '\0';
		printf("%s\n", buf);
		
        int number;long RDI;long RSI;long RDX;long R10;long R8;long R9;

		strtoargs(buf,&number,&RDI,&RSI,&RDX,&R10,&R8,&R9);

		printf("参数：%d,%ld,%ld,%ld,%ld,%ld,%ld\n",number,RDI,RSI,RDX,R10,R8,R9);

		ret = do_syscall(number,RDI,RSI,RDX,R10,R8,R9);
        
        itoa(ret,buf,10);
        
        write(client_socket, buf, ret);
		
	}
	close(client_socket);
}

// 系统调用入口
int do_syscall(int number,long RDI,long RSI,long RDX,long R10,long R8,long R9){
    int ret = syscall(number,RDI,RSI,RDX,R10,R8,R9);
    return ret;
}

int creat_socket()         //创建套接字和初始化以及监听函数
{
	int listen_socket = socket(AF_INET, SOCK_STREAM, 0);      //创建一个负责监听的套接字  
	if(listen_socket == -1)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	
	addr.sin_family = AF_INET;  /* Internet地址族 */
    addr.sin_port = htons(PORT);  /* 端口号 */
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   /* IP地址 */
	
	int ret = bind(listen_socket, (struct sockaddr *)&addr, sizeof(addr));    //连接
	if(ret == -1)
	{
		perror("bind");
		return -1;
	}
	
	ret = listen(listen_socket, 5);   //监听
	if(ret == -1)
	{
		perror("listen");
		return -1;
	}
	return listen_socket;
}
 
int wait_client(int listen_socket)
{
	struct sockaddr_in cliaddr;
	int addrlen = sizeof(cliaddr);
	printf("等待客户端连接。。。。\n");
	int client_socket = accept(listen_socket, (struct sockaddr *)&cliaddr, &addrlen);     //创建一个和客户端交流的套接字
	if(client_socket == -1)
	{
		perror("accept");
		return -1;
	}
	
	printf("成功接收到一个客户端：%s\n", inet_ntoa(cliaddr.sin_addr));
	
	return client_socket;
}
 

long  atol(const char *nptr)
{
        int c;              /* 当前要转换的字符(一个一个字符转换成数字) */
        long total;         /* 当前转换结果 */
        int sign;           /* 标志转换结果是否带负号*/
 
        /*跳过空格，空格不进行转换*/
        while ( isspace((int)(unsigned char)*nptr) )
            ++nptr;
 
        c = (int)(unsigned char)*nptr++;//获取一个字符准备转换 
        sign = c;           /*保存符号标示*/
        if (c == '-' || c == '+')
            c = (int)(unsigned char)*nptr++;    /*跳过'+'、'-'号，不进行转换*/
 
        total = 0;//设置转换结果为0 
 
        while (isdigit(c)) {//如果字符是数字 
            total = 10 * total + (c - '0');     /* 根据ASCII码将字符转换为对应的数字，并且乘10累积到结果 */
            c = (int)(unsigned char)*nptr++;    /* 取下一个字符 */
        }
 
         //根据符号指示返回是否带负号的结果 
        if (sign == '-')
            return -total;
        else
            return total;  
}

int  atoi(const char *nptr)
{
        return (int)atol(nptr);
}

//把字符串中的参数解析出来
void strtoargs(char* buf,int* number,long* RDI,long* RSI,long* RDX,long* R10,long* R8,long* R9){

	int len = strlen(buf);
	for(int i=0 ; i< len;i++){
		if(buf[i] == ','){
			buf[i] = 0;
		}
	}

	*number = atoi(buf);
	
	int last_len = strlen(buf);
	buf[last_len] = ',';
	*RDI = atoi(buf + last_len + 1);

	last_len = strlen(buf);
	buf[last_len] = ',';
	*RSI = atoi(buf + last_len + 1);

	last_len = strlen(buf);
	buf[last_len] = ',';
	*RDX = atoi(buf + last_len + 1);

	last_len = strlen(buf);
	buf[last_len] = ',';
	*R10 = atoi(buf + last_len + 1);

	last_len = strlen(buf);
	buf[last_len] = ',';
	*R8 = atoi(buf + last_len + 1);

	last_len = strlen(buf);
	buf[last_len] = ',';
	*R9 = atoi(buf + last_len + 1);
}

char* itoa(int num,char* str,int radix)
{
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//索引表
    unsigned unum;//存放要转换的整数的绝对值,转换的整数可能是负数
    int i=0,j,k;//i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。
 
    //获取要转换的整数的绝对值
    if(radix==10&&num<0)//要转换成十进制数并且是负数
    {
        unum=(unsigned)-num;//将num的绝对值赋给unum
        str[i++]='-';//在字符串最前面设置为'-'号，并且索引加1
    }
    else unum=(unsigned)num;//若是num为正，直接赋值给unum
 
    //转换部分，注意转换后是逆序的
    do
    {
        str[i++]=index[unum%(unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
        unum/=radix;//unum去掉最后一位
 
    }while(unum);//直至unum为0退出循环
 
    str[i]='\0';//在字符串最后添加'\0'字符，c语言字符串以'\0'结束。
 
    //将顺序调整过来
    if(str[0]=='-') k=1;//如果是负数，符号不用调整，从符号后面开始调整
    else k=0;//不是负数，全部都要调整
 
    char temp;//临时变量，交换两个值时用到
    for(j=k;j<=(i-1)/2;j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
    {
        temp=str[j];//头部赋值给临时变量
        str[j]=str[i-1+k-j];//尾部赋值给头部
        str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
    }
 
    return str;//返回转换后的字符串
 
}