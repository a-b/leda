T= lfs

# Compilation directives

LUA_INC =   ../luajit/src
INCS= -I$(LUA_INC)
CC= gcc


TARGET = lfs.so

CFLAGS = -O2 -Wall 

OBJS= lfs.o
BUILD_CFLAGS =  $(CFLAGS) $(INCS)

PLATFORM_LDFLAGS ?= 

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
PLATFORM_LDFLAGS = -shared -fPIC
endif

ifeq ($(UNAME), Darwin)
PLATFORM_LDFLAGS = -bundle -undefined dynamic_lookup
endif

.PHONY: all clean 
	
.c.o:
	$(CC) -c $(BUILD_CFLAGS) -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJS)
	env $(CC) $(PLATFORM_LDFLAGS) $(OBJS) -o $(TARGET)
	

clean:
	rm -f $(TARGET) $(OBJS) 
	rm -f $(TARGET) *.so
	rm -f $(TARGET) *.a
	
	
	
