include ../../system.mk

watchdog_test:
	arm-mol-linux-uclibcgnueabihf-gcc watchdog_test.c -static -o watchdog_test
	arm-mol-linux-uclibcgnueabihf-gcc watchdog_alive_test.c -static -o watchdog_alive_test
	cp watchdog_test watchdog_alive_test $(APP_OUTPUT_DIR)

clean:
	rm -f *.o watchdog_test watchdog_alive_test
