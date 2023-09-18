################################################################################
# Automatically-generated file. Do not edit!
################################################################################
#-march=armv7-a -mfloat-abi=softfp -mfpu=neon-vfpv4 -mno-unaligned-access -fno-aggressive-loop-optimizations
include ../Makefile.pub 
INCLUDES=$(wildcard *.h)
DEFS = 
CFLAGS :=   -Wall   $(DEFS)  
CFLAGS +=  -I./     -I../include -I./common -I./osal -I../connet -I../sqlite3/output/include
CXXFLAGS:=$(CFLAGS)
CCFLAGS:=$(CFLAGS)
LIBPATH :=  -L../lib -L../sqlite3/output/lib
LIBS := -lsqlite3  -l:libmpi.a -l:libaudio_codec.a -lpthread -lm -ldl  ./osal/osal.a ./connet.a

##### End of variables to change
SOURCES =  $(wildcard *.c common/*.c) 
OBJFILES = $(SOURCES:%.c=obj/%.o)

.PHONY :clean install prepare all
TARGET = dec_bin
all : prepare $(TARGET) 


#change .cpp files  to .o files


#$@--目标文件，$^--所有的依赖文件，$<--第一个依赖文件。

obj/%.o : %.c $(INCLUDES)
	$(COMPILE_CC)
	
obj/common/%.o : common/%.c
	$(COMPILE_CC)	

dec_bin:$(OBJFILES) 
	$(LINK_CC)  
	
prepare:
	@echo "preparing..."
	@if ! [ -d obj ]; then mkdir obj; fi;
	@if ! [ -d obj/common ]; then mkdir obj/common; fi;
	

clean:
	-rm -rf *.o obj/*.o $(TARGET) *~ obj/

install:
	cp $(TARGET) /home/work/nfs/nvr


##### Any additional, platform-specific rules come here:

