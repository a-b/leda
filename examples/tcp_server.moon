-- example that starts TCP server on port 8500, that sends "hello" to every connected client every second and that echoes all  input it receives

require 'leda.tcp_server'

class ExampleServer extends TCPServer

    -- port 8500
    port: 8500

    -- set number of threads to 4
    threads: 4

    onDataReceived: (connection, data) =>
        -- send data back on connection
        connection\send(data)

    onThreadStarted: (thread) =>
        print "started thread ", thread.id

        callback = -> 
            for id, connection in pairs @connections
                connection\send("hello") 

        self\setTimer(1, callback)    
        
    onConnectionOpened: (connection) =>
        -- call base class method
        super(connection)    
        
        print string.format("accepted  new client connectionid  %s from %s", connection\id!, connection\address!)
        
    onConnectionClosed: (connection) =>
        -- call base class method
        super(connection)    
    
        print string.format("closed client connectionid  %s", connection\id!)    
        

ExampleServer!    