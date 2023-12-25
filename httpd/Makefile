include ../Makefile.pub

INCLUDES=$(wildcard *.h)
DEFS =
CFLAGS := -Wall $(DEFS) -I.  -g 
CFLAGS +=  -I./   -I../connet/ -I../connet/util -I../sqlite3/output/include
CXXFLAGS:=$(CFLAGS)
CCFLAGS:=$(CFLAGS)
LIBPATH := -L../sqlite3/output/lib
LIBS := -lsqlite3 -lpthread ../connet/connet.a
INSTALL_DIR=/home/work/nfs/nvr

IMG_DIR=/home/work/fhw/main/user_fhw

##### End of variables to change
.PHONY :prepare all clean install

TARGET = httpd_bin
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
	$(STRIP) $(TARGET)
	install $(TARGET) $(INSTALL_DIR)
	install $(TARGET) $(IMG_DIR)
##### Any additional, platform-specific rules come here:
