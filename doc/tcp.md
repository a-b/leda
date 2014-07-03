TCP
====

To add functionality of a generic TCP server one has to include `leda.tcp` module, create new  Server object and pass callback function via the `data` method. Note that callbacks can block and does not have to contain asynchronous code.

Server objects are created by calling the Server() function

**Server(port, host)**: creates and starts up a new generic TCP server listening for incoming connections on specified host and port. If host is not provided it defaults to INADDR_ANY (listen for connections on all interfaces)


## Server objects

Fields:

**port**: listening port. 

**host**: listening host

**threads**: number of connection threads. defaults to number of processors

**connections**: contains a table of active connections that maps connection id to connection objects

Methods:

**data(callback)**: function provided in the `callback` is invoked whenever  server processes an incoming data on one of the connections. This function arguments are connnection object and data received.

**connection(callback)**: function provided in the `callback` is invoked whenever new connection is accepted by server. This function arguments are connnection object and a boolean flag whether connection was opened or closed

**thread(callback)**: function provided in the `callback` is invoked whenever new connection thread is started or stopped. This function arguments are thread object and a boolean flag whether thread was started or stopped

**timer(timeout, callback)**: calls the timer to call the function provided in the  `callback` parameter every number of seconds specified by `timeout` value

**timeout(timeout, callback)**: calls the function provided in the  `callback` parameter once after the number of seconds provided in the `timeout` parameter passes


## Connection objects

Methods:

**id()**: get connection id

**address()**: get connection remote address

**send(data)**: send data over the connection


## Thread objects

Fields:

**id**: thread id

## Example

Consider the following example that creates TCP server listening on localhost, port 12000 and has the following logic: it sends back all data it receives and sends string "hello" to all open connections every second

        local tcp = require('leda.tcp')

        local server = tcp.Server(12000, 'localhost')

        server:data(function(connection, data)
            -- send back data received 
            connection:send(data)
            end)

        server:thread(function(thread, started) 
            -- set the timer to send data to all opened connections
            server:timer(1, function() 
                for id, connection in pairs(server.connections) do
                    connection:send('hello')
                end
            end)
        end)    
        
        