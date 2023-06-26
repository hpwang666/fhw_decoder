include ../../system.mk

test:
	arm-mol-linux-uclibcgnueabihf-gcc dma_ioctl_test.c -static -o dma_ioctl_test
	cp dma_ioctl_test $(APP_OUTPUT_DIR)

clean:
	rm -f *.o dma_ioctl_test
