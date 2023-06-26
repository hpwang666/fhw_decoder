#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>




int main(int argc, char **argv)
{
	unsigned int loop = 0;
	unsigned int klog = 0;
	unsigned int ulog = 0;
	char *buf[1] = {"1"};
	int fd;
	unsigned int i;
	char *stop;

	if (argc != 5 ) {
		printf("parameters: [kernel log : 0, 1] [lookat path] [user log : 0, 1] [loop times]\n");
		return 0;
	}

	klog = strtol(argv[1], &stop, 2);
	if (klog) {
		fd=open(argv[2], O_RDWR);
		if(fd < 0)
			printf("open %s error\n", argv[1]);
	}
	ulog = strtol(argv[3], &stop, 2);
	loop = strtol(argv[4], &stop, 10);
	for(i = 0; i < loop; i++) {
		if (ulog)
			printf("i=%d\n",i);

		if (klog)
			write (fd, buf, 1);

		if (ulog) {
			printf("<loop for -uart check %d times--------------->\n", i);
			printf("<1234567890abcdefghijklmnopqrstuvwxyzABUCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABUCDEFGHIJKLMNOPQRSTUVWXYZ>\n");
		}
	}

	close(fd);

	return 0;
}
