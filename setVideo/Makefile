include ../Makefile.pub
INCLUDES = -I ./
		
##### Change the following for your environment: 
CXXFLAGS =	$(COMPILE_OPTS) -Wall -D_FILE_OFFSET_BITS=64 -fPIC -std=c++11

##### End of variables to change
ALL = prepare $(TARGET) 
TARGET = setVideo_bin
all:  $(ALL)


SOURCES =  $(wildcard *.cpp) 

#change .cpp files  to .o files
OBJFILES = $(SOURCES:%.cpp=obj/%.o)

LIBPATH := 
LIBS := -lpthread
INSTALL_DIR=

obj/%.o:%.cpp    
	$(COMPILE_CXX)


#不会删除中间文件
.SECONDARY:
	
$(TARGET):	$(OBJFILES) 
	@echo "linking $(TARGET) from ..."
	$(LINK_CXX)
	

prepare:
	@echo "preparing..."
	@if ! [ -d obj ]; then mkdir obj; fi;

clean:
	-rm -rf *.o obj/*.o $(ALL) *~ 
install:
	install $(TARGET) $(INSTALL_DIR)
	
.PHONY :clean install prepare all

##### Any additional, platform-specific rules come here:
