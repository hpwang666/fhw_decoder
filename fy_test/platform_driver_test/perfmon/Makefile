include ../../system.mk

test:
	arm-mol-linux-uclibcgnueabihf-gcc perfmon.c -static -o perfmon
	cp perfmon $(APP_OUTPUT_DIR)


clean:
	rm -f *.o perfmon
