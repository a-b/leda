# -------------------------------------------------------------------------
    # These are configurable options:
# -------------------------------------------------------------------------

# 'install' program location 
INSTALL ?= install


# Destination root (/ is used if empty) 
DESTDIR ?= 

CC = gcc

# C++ compiler 
CXX = g++

# Standard flags for C++ 
CXXFLAGS ?= 

# Standard preprocessor flags (common for CC and CXX) 
CPPFLAGS ?= -DUSE_CJSON -O2

# Standard linker flags 
LDFLAGS ?= 

PLATFORM_LDFLAGS ?= 

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
PLATFORM_LDFLAGS = -pthread  -lrt -ldl -lm -export-dynamic
endif

ifeq ($(UNAME), Darwin)
PLATFORM_LDFLAGS = -pagezero_size 10000 -image_base 100000000 -framework CoreServices
endif

include config.mk

# Location where the package is installed by 'make install' 
prefix ?= $(DEST)



### Variables: ###

CPPDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP
LEDA_CXXFLAGS = -Ideps/libpropeller/include -Ideps/libpropeller/deps/libevent/include -Ideps/luajit/src -Ideps/cjson -DLEDA_PATH=\"$(prefix)/lib/leda\" -D_THREAD_SAFE -pthread \
	$(CPPFLAGS) $(CXXFLAGS) 
		
SOURCES = $(wildcard src/*.cpp)

OBJECTS = $(SOURCES:.cpp=.o)	

TARGET = leda

OPENSSL_LIB ?=

ifeq ($(OPENSSL),yes)
	OPENSSL_LIB=-lssl -lcrypto -levent_openssl
endif

### Targets: ###

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
	-(cd deps/cjson && $(MAKE) clean)
	
libs: 
	cd deps/libpropeller && make && cd ../luajit  && make && rm src/libluajit.so  && cd ../cjson && make


$(TARGET): libs $(OBJECTS) 
	$(CXX) -o $@ $(OBJECTS)  -Ldeps/libpropeller -Ldeps/libpropeller/deps/libevent/.libs -Ldeps/luajit/src -Ldeps/cjson  $(LDFLAGS) \
	     -lpropeller -lluajit   -levent -levent_pthreads  -lcjson   $(PLATFORM_LDFLAGS) $(OPENSSL_LIB)

install_leda: 
	$(INSTALL) -d $(DESTDIR)$(prefix)/bin
	install -c $(TARGET) $(DESTDIR)$(prefix)/bin
	rm -rf $(DESTDIR)$(prefix)/lib/leda
	mkdir -p $(DESTDIR)$(prefix)/lib/leda
	rsync -a --exclude='test' --exclude 'test.lua' lib/* $(DESTDIR)$(prefix)/lib/leda/
	cd deps/luajit && make install
	chmod -R a+r $(DESTDIR)$(prefix)/lib/leda/
	chmod -R a+r $(DESTDIR)$(prefix)/lib/lua/5.1/
	
	@echo $(TARGET) has been installed to $(prefix)

uninstall_leda: 
	rm -f $(DESTDIR)$(prefix)/bin/leda
	
doc:
	ldoc -p leda -d doc lib/leda	
	
test:
	test/run.sh	

%.o: %.cpp
	$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<


.PHONY: all install uninstall clean install_leda uninstall_leda doc test


# Dependencies tracking:
-include src/*.d
