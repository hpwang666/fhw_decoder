#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/watchdog.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd, timeout, feed_time;
	if(argc < 3)
	{
		printf("userge:<timeout:NUMs> <how long feed dog:NUMs>\r\n");
		return 0;
	}

	timeout = atoi(argv[1]);
	feed_time = atoi(argv[2]);

	fd = open("/dev/watchdog", O_WRONLY);
	if(fd == -1){
		printf("open watchdog error \n\n\n");
		return 0;
	}

	ioctl(fd, WDIOC_SETTIMEOUT, &timeout); //设置超时
	printf("The timeout was set to %d seconds\n", timeout);
	printf("The watchdogs were fed every %d seconds\n", feed_time);
	while(1) {

		ioctl(fd, WDIOC_KEEPALIVE, &timeout, NULL);
		sleep(feed_time);
	}
	return 1;
}
