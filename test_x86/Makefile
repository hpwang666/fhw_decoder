include ~/Makefile.pub

INCLUDES=$(wildcard *.h)
DEFS =
CFLAGS := -Wall $(DEFS) -I.  -g
CFLAGS +=  -I./    -Iconn/
CXXFLAGS:=$(CFLAGS)
CCFLAGS:=$(CFLAGS)
LIBPATH := 
LIBS := -lpthread ./conn/conn.a
INSTALL_DIR=



##### End of variables to change
.PHONY :prepare all clean install



SOURCES =  $(wildcard *.c) 

#change .cpp files  to .o files
OBJFILES = $(SOURCES:%.c=obj/%.o)
TARGET = $(SOURCES:%.c=%_bin)
ALL = $(TARGET)
all: prepare $(ALL)

.SECONDARY:
	
obj/%.o:%.c  $(INCLUDES)
	$(COMPILE_CC)
	
	
%_bin:obj/%.o
	$(LINK_CC)
prepare:
	@echo "preparing..."
	@if ! [ -d obj ]; then mkdir obj; fi;
clean:
	-rm -rf *.o $(ALL) *~ obj/* obj
install:
	install $(TARGET) $(INSTALL_DIR)

##### Any additional, platform-specific rules come here:
