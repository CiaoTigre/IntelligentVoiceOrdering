
#common makefile header


DIR_INC = include
DIR_LIB = libs

TARGET	= demo_iat_tts
BIN_TARGET = $(BUILDDIR)/$(TARGET)

#CROSS_COMPILE = 
CFLAGS = -g -Wall -I$(DIR_INC)      # C语言编译选项

ifdef LINUX64
LDFLAGS := -L$(DIR_LIB)/x64
else
LDFLAGS := -L$(DIR_LIB)/x86 
endif
LDFLAGS += -lmsc -lrt -ldl -lpthread -lasound -lstdc++    # 链接库选项


OBJS      := $(patsubst %.c,%.o,$(wildcard *.c))
BUILDDIR   = ./build
OBJECTDIR  = $(BUILDDIR)/objs
BUILDOBJS  = $(foreach c, $(OBJS), $(subst $(c), $(OBJECTDIR)/$(c), $(c)))

#--------------------------------------------------------------------------
build : prepare $(TARGET)
prepare :
	mkdir -p $(BUILDDIR)
	mkdir -p $(OBJECTDIR)

#------------------------------------------------------------- $@ 表示目标文件
$(OBJECTDIR)/%.o : %.c                                                 
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@     
	
	
	
$(TARGET) : $(BUILDOBJS)
	$(CROSS_COMPILE)gcc $(CFLAGS) $^ -o $(BUILDDIR)/$@ $(LDFLAGS)



#----------------------------------------------------------------------------
clean:
	@rm -f $(OBJECTDIR)/*.o $(BIN_TARGET) *.o

.PHONY:clean

#common makefile foot
