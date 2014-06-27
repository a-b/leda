TCP
====

To add functionality of a generic TCP server one has to create new instance of `TCPServer` class with several fields initialized

## `TCPServer` class

Fields:

**port**: listening port. defaults to 8080

**host**: hostname or ip that server binds to. defaults to 'local'. empty means accept all incoming connections

**on_data**: function that gets invoked whenever http server processes an incoming request. This function is passed three arguments: server object ,connnection object and data received

**on_connection**: if set to a function, it gets invoked whenever new connection is accepted by server. This function is passed three arguments: server object , connnection object and a boolean flag whether connection was opened or closed

**on_thread**: if set to a function, it gets invoked whenever new connection thread is started or stopped. This function is passed three arguments: server object , thread object and a boolean flag whether thread was started or stopped

**threads**: number of connection threads. defaults to 1

**connections**: contains a table of active connections that maps connection id to connection objects

Methods:

**start()**: starts up the TCP server

**setTimer(timeout, callback)**: calls the timer to call the function provided in the  `callback` parameter every number of seconds specified by `timeout` value

**setTimeout(timeout, callback)**: calls the function provided in the  `callback` parameter once after the number of seconds provided in the `timeout` parameter passes


## Connection objects

Connection objects have following fields:

**id**: connection id

Methods:

**address()**: get connection address

**send(data)**: send data over the connection


## Thread objects

Response objects have following fields:

**id**: thread id

## Example

Consider the following example that creates TCP server listening on port 12000 and has the following logic: it sends back all data it receives and sends string "hello" to all open connections every second

        require 'leda.tcp_server'
        
        with TCPServer!
            .port =  12000
            .on_data = (server, connection, data) ->
                connection\send(data)
        
            .on_thread = (server, thread, started) ->
                if started
                    server\setTimer 1, ->
                        for id, connection in pairs server.connections
                            connection\send("hello")
                
            \start!    
        