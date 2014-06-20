leda
====

high performance framework for creating powerful network applications with leightweight scripting

Features:

* Synchronous scripting with moonscript (http://www.moonscript.org)
* Super fast VM with LuaJIT(http://luajit.org)
* Multithreaded process architecture and asynchronous networking with libevent (via https://github.com/sergeyzavadski/libpropeller)

Installing:

    $ git clone https://github.com/sergeyzavadski/leda.git
    $ cd leda
    $ ./configure
    $ make
    $ sudo make install
    

Running:

To run an application that starts server TCP is listening on port 8000 and sends back all data it receives as well as sends "hello" every second to cononections:

Edit `server.moon` looks like this:

	class MyServer extends TCPServer
    
        onDataReceived: (connection, data) =>
            -- send data back on connection
            connection\send(data)
        
	
        onThreadStarted: (thread) =>
            callback = -> 
                for id, connection in pairs @connections
                    connection\send("hello") 
            
            self\setTimer(1, callback)    
    
  
    MyServer!    

Run

    $ leda server.moon


  
		

