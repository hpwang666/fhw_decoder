include ../../system.mk

test:
	arm-mol-linux-uclibcgnueabihf-gcc uart_test.c -static -o uart_test
	cp uart_test $(APP_OUTPUT_DIR)

clean:
	rm -f *.o uart_test

