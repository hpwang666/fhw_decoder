include ../Makefile.pub

DEFS =
CFLAGS := -Werror -g -funsigned-char $(DEFS) -I.
CYGWIN := $(shell uname -a | grep -o Cygwin)
CFLAGS += -fPIC
CFLAGS += -I../../libevent/include 
CXXFLAGS:=$(CFLAGS)
CCFLAGS:=$(CFLAGS)
LIBPATH := 
LIBS :=




##### End of variables to change
.PHONY :prepare all clean install

TARGET = spawn
ALL = $(TARGET)
all: $(ALL)

SOURCES =  $(wildcard *.c) 

#change .cpp files  to .o files
OBJFILES = $(SOURCES:%.c=%.o)

%.o:%.c 
	$(COMPILE_CC)
	
$(TARGET):	$(OBJFILES)  
	$(call ASSEMBLE_EXTRA,CC)
prepare:
	@echo "preparing..."

clean:
	-rm -rf *.o $(ALL) *~ 
install:
	install $(TARGET) $(INSTALL_DIR)

##### Any additional, platform-specific rules come here:
