include ../Makefile.pub

INCLUDES=$(wildcard *.h)
DEFS =
CFLAGS := -Wall $(DEFS) -I.  -g 
CFLAGS +=  -I./   -I../connet/ -I../zlog/output/include -I../connet/util
CXXFLAGS:=$(CFLAGS)
CCFLAGS:=$(CFLAGS)
LIBPATH := -L../zlog/output/lib
LIBS := -lpthread -lzlog ../connet/connet.a
INSTALL_DIR=/home/work/nfs/nvr


##### End of variables to change
.PHONY :prepare all clean install

TARGET = rtsp_bin
ALL = $(TARGET)
all: prepare $(ALL)

SOURCES =  $(wildcard *.c) 

#change .cpp files  to .o files
OBJFILES = $(SOURCES:%.c=obj/%.o)

obj/%.o:%.c  $(INCLUDES)
	$(COMPILE_CC)
	
	
$(TARGET):	$(OBJFILES)  
	$(LINK_CC)
prepare:
	@echo "preparing..."
	@if ! [ -d obj ]; then mkdir obj; fi;
clean:
	-rm -rf *.o $(ALL) *~ obj/* obj
install:
	install $(TARGET) $(INSTALL_DIR)

##### Any additional, platform-specific rules come here:
