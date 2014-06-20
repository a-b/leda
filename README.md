leda
====

high performance framework for creating powerful mnetwork applications with leightweight scripting

Features:

* Synchronous scripting with moonscript (http://www.moonscript.org)
* Super fast VM with LuaJIT(http://luajit.org)
* Multithreaded process architecture and asynchronous networking with libevent

Example:

	application that starts server TCP listening on se port 8000 that sends back all data it receives and sends "hello" every second to cooonections:

	class MyServer extends Server
    onDataReceived: (connection, data) =>
	-- send data back on connection
        connection\send(data)
        
	
    onThreadStarted: (thread) =>
        callback = -> 
            for id, connection in pairs @connections
                connection\send("hello") 
            
        self\setTimer(1, callback)    
    
  

MyServer!    

  
		

