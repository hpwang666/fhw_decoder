#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h> //文件控制定义  
#include <termios.h>//终端控制定义  
#include <errno.h>  
#include<sys/ioctl.h>
 
#define DEVICE "/dev/ttyS1"  
  
#undef  _DEBUG
//#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif

int initSerial(void)  
{  
    int serial_fd;
	serial_fd = open(DEVICE, O_RDWR| O_NOCTTY); // | O_NDELAY 
    if (serial_fd < 0) {  
        perror("open");  
        return -1;  
    }  
      
    //串口主要设置结构体termios <termios.h>  
    struct termios options;  
    tcgetattr(serial_fd, &options);  
 
#if 1
    options.c_cflag |= (CLOCAL | CREAD);//设置控制模式状态，本地连接，接收使能  
    options.c_cflag &= ~CSIZE;//字符长度，设置数据位之前一定要屏掉这个位  
    options.c_cflag &= ~CRTSCTS;//无硬件流控

    options.c_cflag |= CS8;//8位数据长度  
    options.c_cflag &= ~CSTOPB;//1位停止位  
    options.c_iflag &= ~PARENB;//|= IGNPAR;//无奇偶检验位  
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  // 0 /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/              // 0
	cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);//设置波特率  
      
    /**3. 设置新属性，TCSANOW：所有改变立即生效*/  
    tcflush(serial_fd, TCIFLUSH);//溢出数据可以接收，但不读  
    tcsetattr(serial_fd, TCSANOW, &options);  
#endif      
    return serial_fd;  
}  
  

int uartSend(int serial_fd, char *data, int datalen)  
{  
    int len = 0;  
    len = write(serial_fd, data, datalen);
    if(len == datalen) {  
        return len;  
    } else {  
        tcflush(serial_fd, TCOFLUSH); 
        return -1;  
    }   
    return 0;  
}  
  

int serialRecv(int serial_fd,char *data)  
{  
    int len=0;  
	struct timeval timeout;
	 
    fd_set fs_read;  
    FD_ZERO(&fs_read);  
    FD_SET(serial_fd, &fs_read);  
    timeout.tv_sec = 2; 
	
	select(serial_fd+1, &fs_read, NULL, NULL, &timeout);  
	if (FD_ISSET(serial_fd, &fs_read)) 
	{  
		ioctl(serial_fd,FIONREAD,&len);
		len = read(serial_fd, data, 512);
		debug("len[%d]:%d %d %d %d\n\r",len,data[0],data[1],data[2],data[3]);
		return len;
	}
	return 0;
}  
  
  
  
int closeSerial(int serial_fd)
{
	close(serial_fd);
	return 0;
}
