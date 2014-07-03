# -------------------------------------------------------------------------
    # These are configurable options:
# -------------------------------------------------------------------------

# 'install' program location 
INSTALL ?= install

# Location where the package is installed by 'make install' 
prefix ?= /usr/local

# Destination root (/ is used if empty) 
DESTDIR ?= 

CC = gcc

# C++ compiler 
CXX = g++

# Standard flags for C++ 
CXXFLAGS ?= 

# Standard preprocessor flags (common for CC and CXX) 
CPPFLAGS ?= 

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


### Variables: ###

CPPDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP
LEDA_CXXFLAGS = -Ideps/libpropeller/include -Ideps/libpropeller/deps/libevent/include -Ideps/luajit/src -Ideps/cjson -g -D_DEBUG -D_THREAD_SAFE -pthread \
	$(CPPFLAGS) $(CXXFLAGS) 
		
LEDA_OBJECTS =  \
	obj/LEDA_Leda.o \
	obj/LEDA_main.o \
	obj/LEDA_trace.o \
	obj/LEDA_LuaState.o  \
	obj/LEDA_Server.o \
	obj/LEDA_HttpServer.o \
 	obj/LEDA_Client.o \
	obj/LEDA_FWatcher.o
	

### Conditionally set variables: ###

all: obj
obj:
	@mkdir -p obj

### Targets: ###

all: obj/leda

install: install_leda

uninstall: uninstall_leda

clean: 
	rm -f obj/*.o
	rm -f obj/*.d
	rm -f obj/leda
	-(cd deps/libpropeller && $(MAKE) clean)
	-(cd deps/luajit && $(MAKE) clean)
	-(cd deps/cjson && $(MAKE) clean)
	
libs: 
	cd deps/libpropeller && make && cd ../luajit  && make && rm src/libluajit.so  && cd ../cjson && make


obj/leda: libs $(LEDA_OBJECTS) 
	$(CXX) -o $@ $(LEDA_OBJECTS)  -Ldeps/libpropeller/obj -Ldeps/libpropeller/deps/libevent/.libs -Ldeps/luajit/src -Ldeps/cjson  $(LDFLAGS) \
	     -lpropeller -lluajit   -levent -levent_pthreads  -lcjson   $(PLATFORM_LDFLAGS)

install_leda: 
	$(INSTALL) -d $(DESTDIR)$(prefix)/bin
	install -c obj/leda $(DESTDIR)$(prefix)/bin
	rm -rf $(DESTDIR)$(prefix)/lib/leda
	mkdir -p $(DESTDIR)$(prefix)/lib/leda
	rsync -a --exclude='test' --exclude 'test.lua' lib/* $(DESTDIR)$(prefix)/lib/leda/
	cd deps/luajit && make install
	chmod -R a+r $(DESTDIR)$(prefix)/lib/leda/
	chmod -R a+r $(DESTDIR)$(prefix)/lib/lua/5.1/

uninstall_leda: 
	rm -f $(DESTDIR)$(prefix)/bin/leda

obj/LEDA_Leda.o: src/Leda.cpp
	$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<

obj/LEDA_LuaState.o: src/LuaState.cpp
	$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<
	
obj/LEDA_Client.o: src/Client.cpp
	$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<	

obj/LEDA_Server.o: src/Server.cpp
		$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<	
	
obj/LEDA_HttpServer.o: src/HttpServer.cpp
		$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<		

obj/LEDA_main.o: src/main.cpp
	$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<

obj/LEDA_trace.o: src/trace.cpp
	$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<
	
obj/LEDA_FWatcher.o: src/FWatcher.cpp
	$(CXX) -c -o $@ $(LEDA_CXXFLAGS) $(CPPDEPS) $<
	


.PHONY: all install uninstall clean install_leda uninstall_leda


# Dependencies tracking:
-include obj/*.d
