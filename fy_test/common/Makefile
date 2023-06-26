##### Change the following for your environment: 
CROSS_COMPILE =		arm-mol-linux-uclibcgnueabihf-
COMPILE_OPTS =	 -O2 
C =			c
C_COMPILER =		$(CROSS_COMPILE)gcc
C_FLAGS =		 -I./    -I../test/common -I../../include
CPP =			cpp
CPLUSPLUS_COMPILER =	$(CROSS_COMPILE)gcc -c
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -DBSD=1
OBJ =			o
LINK =			$(CROSS_COMPILE)gcc -o 
LINK_OPTS =	-lpthread	
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		$(CROSS_COMPILE)ar cr 
LIBRARY_LINK_OPTS =	$(LINK_OPTS)
LIB_SUFFIX =			a

##### End of variables to change

MEDIA_SERVER = libfycommon.a

ALL = $(MEDIA_SERVER)
all: $(ALL)

SOURCES = $(wildcard *.c) 
HEADERS = $(wildcard *.h) 

#change .cpp files  to .o files
OBJFILES = $(SOURCES:%.c=%.o)

$(OBJFILES):	%.o:%.c   $(HEADERS) 
	@echo Compiling $@ from $<..
	$(C_COMPILER)  $(C_FLAGS) -c -o $@ $<


$(MEDIA_SERVER):	$(OBJFILES) 
	@echo " linking sos from ........ "
#	$(LINK)$@ $(CONSOLE_LINK_OPTS) $(OBJFILES)
	$(LIBRARY_LINK)$@  $(OBJFILES)

clean:
	-rm -rf *.$(OBJ) $(ALL) *~ 


##### Any additional, platform-specific rules come here:
