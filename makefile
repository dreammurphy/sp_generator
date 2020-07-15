#这是一个makefile的参考样例,用于编译多文件夹,多文件
#编译工具链
CC	= gcc
CXX	= g++
LD	= $(CC)
LINK = g++
LIBS = -lz -lm -lpcre
AS	= nasm

#编译选项
ASFLAGS  = -m64
CFLAGS   = -m64 -c -Wall $(incdir)	 
CXXFLAGS = -m64 -c -Wall $(incdir)
#CFLAGS   = -m32 -Wall $(incdir) -c -g -fPIC
#CXXFLAGS = -m32 -Wall $(incdir) -c -g -fPIC
LDFLAGS	 = -m64 -static

#定义输出文件名
target = main.exe
TARGET = main.exe
TARGETP = mainp.exe

#定义根目录
objdir = obj
srcdir = .

VPATH = .:./utils

#定义源码子目录
subdir = . \
         utils \


#定义包含目录
incdir = $(foreach dir, $(subdir), -I$(srcdir)/$(dir))
INCLUDES = $(foreach dir, $(subdir), -I$(srcdir)/$(dir))


#定义附加依赖库
inclib = -lpthread

#遍历所有dir,并搜索该dir下面的所有.c文件
allsrc = $(foreach dir, $(subdir), $(wildcard $(srcdir)/$(dir)/*.c))
allsrcpp = $(foreach dir, $(subdir), $(wildcard $(srcdir)/$(dir)/*.cpp)) 
allobj2 = $(foreach dir, $(subdir), $(wildcard $(srcdir)/$(dir)/*.o)) 

##把所有的.c替换为.o
#allobj = $(patsubst $(srcdir)/%.c, $(objdir)/%.o, $(allsrc)) 
allobj = $(patsubst $(srcdir)/%.cpp, $(objdir)/%.o, $(allsrcpp)) 
#
##把所有的.c替换为.d
#alldep = $(patsubst $(srcdir)/%.c, $(objdir)/%.d, $(allsrc))
alldep = $(patsubst $(srcdir)/%.cpp, $(objdir)/%.d, $(allsrcpp))
#
#生成.d的规则
$(objdir)/%.d: $(srcdir)/%.cpp 
	@echo Generating $@...
	@$(CC) -MM -MT "$(patsubst $(srcdir)/%.cpp, $(objdir)/%.o, $<) $@" $(CXXFLAGS) $< >$@
#	
##生成.o的规则
$(objdir)/%.o: $(srcdir)/%.cpp
	@echo Compiling $<...
	@$(CXX) $(CXXFLAGS) -c $< -o $@


OBJFILE = $(allsrcpp:.cpp=.o) 
#OBJFILE = $(objdir)/%.o

all:$(TARGET)
	
$(TARGET): $(OBJFILE)
#	$(LINK) $^ $(LIBS) -Wall -fPIC -shared -o $@
	$(LINK) $^  -Wall -o $@

#%.o:%.c
#	$(CC) -o $@ $(CFLAGS) $< $(INCLUDES)
#
#%.o:%.cpp
#	$(CXX) -o $@ $(CXXFLAGS) $< $(INCLUDES)

 
	
#如果是第3次调用make,则包含所有.d文件	
#ifeq ($(MAKELEVEL), 2)
#include $(alldep)
#endif
	
.PHONY : clean：p
clean:
	@echo $(OBJFILE)
#	@-del -rf $(TARGET)
#	@-del -rf $(allobj2)

	del *.o
	del utils\*.o
	@echo clean done.
p:$(TARGETP)
	
$(TARGETP): $(OBJFILE)
#	$(LINK) $^ $(LIBS) -Wall -fPIC -shared -o $@
	$(LINK) $^  -Wall -o -pg -lc $@
