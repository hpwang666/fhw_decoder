#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/watchdog.h>

int main(int argc, char **argv)
{
	int fd = open("/dev/watchdog", O_WRONLY);
	if(fd == -1){
		printf("open watchdog error \n\n\n");
		return 0;
	}
	int timeout;
	timeout = 2;
	ioctl(fd, WDIOC_SETTIMEOUT, &timeout); //设置超时
	printf("The timeout was set to %d seconds\n", timeout);
	//while(1)
		ioctl(fd, WDIOC_KEEPALIVE, &timeout, NULL);
	return 1;
}
