include ../Makefile.pub
  
#-I../logger    -I./net/reactor
DEFS =
CFLAGS :=   -Wall   $(DEFS) 
CFLAGS +=  -I./    -I../connet
CXXFLAGS:=$(CFLAGS)
CCFLAGS:=$(CFLAGS)
LIBPATH := 
LIBS :=   ../connet/connet.a -lm\
			-Wl,-rpath=.
#注意lib的顺序，最底层的放在最后面，以便于被前面的库引用

INSTALL_DIR=/home/work/nfs/nvr
##### End of variables to change
SOURCES =  $(wildcard *.c) 
OBJFILES = $(SOURCES:%.c=obj/test_%.o)

.PHONY :clean  prepare all
TARGET =listen_udp_bin
all : prepare $(TARGET) 



prepare:
	@if ! [ -d obj ]; then mkdir obj; fi;
	
obj/test_%.o:%.c
	$(COMPILE_CC) 

$(TARGET):$(OBJFILES)
	$(LINK_CC)    

clean:
	rm -rf obj  *~ obj/db.o $(TARGET)


install:
	$(STRIP) $(TARGET)
	install $(TARGET) $(INSTALL_DIR)

