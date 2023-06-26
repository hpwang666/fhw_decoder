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
all:nvmem-test

nvmem_src = efuse.c

nvmem_obj = $(nvmem_src:%.c=%.o)

nvmem-test:$(nvmem_obj)
	$(CC) $^ -o $@

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	rm -f *.o nvmem-test
