#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

int main(int argc, char **argv)
{
	int fd,ret;
	char *stop;
	unsigned int dev_addr, data_addr, data;
	struct i2c_rdwr_ioctl_data e2prom_data;
	if (argc != 5 ) {
		printf("parameters: [controller path] [dev_addr] [w] [data_addr] [data]\n");
		return 0;
	}
	/* ---------------------------I2C-0---------------------- */
	fd=open(argv[1], O_RDWR);
	if(fd < 0)
		printf("open %s error\n", argv[1]);
	dev_addr = strtol(argv[2], &stop, 16);
	data_addr = strtol(argv[3], &stop, 16);
	data = strtol(argv[4], &stop, 16);
	printf("open %s, dev_addr [0x%x], data_addr  [0x%x], data [0x%x] \n",
		argv[1], dev_addr, data_addr, data);
	e2prom_data.nmsgs = 2;
	e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
	if (!e2prom_data.msgs) {
		printf("malloc %s error\n", argv[1]);
		exit(1);
	}
	ioctl(fd, I2C_TIMEOUT, 1);/* set time out */
	ioctl(fd, I2C_RETRIES, 2);/* set retry times */

	/* write eeprom */
	e2prom_data.nmsgs = 1;//write opertion need one msg
	(e2prom_data.msgs[0]).len = 2;
	(e2prom_data.msgs[0]).addr = dev_addr; //device address
	(e2prom_data.msgs[0]).flags = 0; //0: write, 1: read
	(e2prom_data.msgs[0]).buf = (unsigned char*)malloc(2);
	(e2prom_data.msgs[0]).buf[0] = data_addr;// data address
	(e2prom_data.msgs[0]).buf[1] = data;// data
	ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
	if (ret < 0)
	printf("ioctl %s write eeprom error\n", argv[1]);

	printf("write %s addr[0x%x] eeprom[0x%x] = 0x%x\n", argv[1],
		    (e2prom_data.msgs[0]).addr,
		    (e2prom_data.msgs[0]).buf[0],
		    (e2prom_data.msgs[0]).buf[1]);
	sleep(1);

	/*read eeprom*/
	e2prom_data.nmsgs = 2;//need two msgs for reading
	(e2prom_data.msgs[0]).len = 1; //first msg for data address
	(e2prom_data.msgs[0]).addr = dev_addr; //device address
	(e2prom_data.msgs[0]).flags = 0;//0: write, 1: read
	(e2prom_data.msgs[0]).buf[0] = data_addr;//data address
	(e2prom_data.msgs[1]).len = 1;//the second msg is for read
	(e2prom_data.msgs[1]).addr = dev_addr;// device address the same as write
	(e2prom_data.msgs[1]).flags = I2C_M_RD;//0: write, 1: read
	(e2prom_data.msgs[1]).buf = (unsigned char*)malloc(1);//data buf
	(e2prom_data.msgs[1]).buf[0] = 0;//init data buf to zero
	ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);//
	if (ret < 0)
	printf("ioctl %s read eeprom error\n", argv[1]);

	printf("read i2c-0 addr[0x%x] eeprom[0x%x] = 0x%x\n",
		    (e2prom_data.msgs[0]).addr,
		    (e2prom_data.msgs[0]).buf[0],
		    (e2prom_data.msgs[1]).buf[0]);

	close(fd);

	return 0;
}
