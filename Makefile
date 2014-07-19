include config.mk
 
INSTALL ?= install

MAKEOVERRIDES = 

CC = gcc

# C++ compiler 
CXX = g++

OPENSSL_LIB ?=

ifeq ($(OPENSSL),yes)
	OPENSSL_LIB=-lssl -lcrypto -levent_openssl
endif

CPPFLAGS ?= -O2

INCLUDES ?= -Ideps/libpropeller/include  -Ideps/luajit/src -Ideps/leveldb/include  -Ideps/libpropeller/deps/libevent/include

# Standard linker flags 
LDFLAGS ?= -Ldeps/libpropeller -Ldeps/luajit/src -Ldeps/leveldb -Ldeps/libpropeller/deps/libevent/.libs  -lpropeller -lluajit -levent -levent_pthreads -lleveldb  $(OPENSSL_LIB)

PLATFORM_LDFLAGS ?= 

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
PLATFORM_LDFLAGS = -pthread  -lrt -ldl -lm -export-dynamic
endif

ifeq ($(UNAME), Darwin)
PLATFORM_LDFLAGS = -pagezero_size 10000 -image_base 100000000 -framework CoreServices
endif

# Location where the package is installed by 'make install' 
prefix ?= $(DEST)

CPPDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP
LEDA_CXXFLAGS = $(INCLUDES)  -DLEDA_PATH=\"$(prefix)/lib/leda\" -D_THREAD_SAFE -pthread  $(CPPFLAGS)
		
SOURCES = $(wildcard src/*.cpp)

OBJECTS = $(SOURCES:.cpp=.o)	

TARGET = leda


all: $(TARGET)

install: install_leda

uninstall: uninstall_leda

clean_leda:
	rm -f src/*.o
	rm -f src/*.d
	rm -f $(TARGET)

clean: 	clean_leda
	-(cd deps/libpropeller && $(MAKE) clean)
	-(cd deps/luajit && $(MAKE) clean)
	-(cd deps/leveldb && $(MAKE) clean)
	
libs: 
	-(cd deps/libpropeller && $(MAKE))
	-(cd deps/luajit && $(MAKE) && rm src/libluajit.so)
	-(cd deps/leveldb && $(MAKE))

$(TARGET): libs $(OBJECTS) 
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) $(PLATFORM_LDFLAGS) 

install_leda: 
	$(INSTALL) -d $(prefix)/bin
	install -c $(TARGET) $(prefix)/bin
	rm -rf $(prefix)/lib/leda
	mkdir -p $(prefix)/lib/leda
	rsync -a --exclude='test' --exclude 'test.lua' lib/* $(prefix)/lib/leda/
	cd deps/luajit && make install
	chmod -R a+r $(prefix)/lib/leda/
	chmod -R a+r $(prefix)/lib/lua/5.1/
	
	@echo $(TARGET) has been installed to $(prefix)

uninstall_leda: 
	rm -f $(prefix)/bin/leda
	
doc:
	ldoc -p leda -d doc lib/leda	
	
test:
	test/run.sh	

%.o: %.cpp
	$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<


.PHONY: all install uninstall clean install_leda uninstall_leda doc test


# Dependencies tracking:
-include src/*.d
