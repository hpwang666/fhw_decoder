include ../Makefile.pub

DEFS =
CFLAGS := -g  $(DEFS) -I. -I../zlog/output/include
CFLAGS += -fPIC 
CXXFLAGS:=$(CFLAGS)
CCFLAGS:=$(CFLAGS)
LIBPATH := -L../zlog/output/lib
LIBS := -lzlog




##### End of variables to change
.PHONY :prepare all clean install

TARGET = smartTemp
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
