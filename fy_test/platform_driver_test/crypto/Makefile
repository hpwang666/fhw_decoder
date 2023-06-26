include ../../system.mk

CROSS_COMPILE := arm-mol-linux-uclibcgnueabihf-

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

srctree := .
objtree	:= .
src		:= $(srctree)
obj		:= $(objtree)
VPATH	:= $(srctree)
prefix ?= ~/fy01_new/linux-fy01/linux-4.9.138
includedir = $(prefix)/include
all:aes-test

crypto_src = cipher.c

crypto_obj = $(crypto_src:%.c=%.o)

aes-test:$(crypto_obj)
	$(CC) $^ -o $@
	cp $@ $(APP_OUTPUT_DIR)

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	rm -f *.o aes-test
