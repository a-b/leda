export ^

class Thread
    new: => 
        @thread = __thread
        @id = __api.threadGetId(@thread)
        
-- connection class
class Connection
    new: =>
        -- assigning via property results in @connection equal nil
        @connection = __connection
    
    address: => __api.serverConnectionGetAddress(@connection) 
    
    id: => __api.serverConnectionGetId(@connection) 
        
    send: (data) =>
        __api.serverConnectionSendData(@connection, data)
        
        
-- server class
class TCPServer
    --default type
    type: "generic"
    -- default port
    port: 8000
    -- default host
    host: '127.0.0.1'
    -- default thread count
    threads: 1
    -- connections map
    connections: {}
    
    -- constructor
    new: =>
        if __init 
            --create server
            __api.serverCreate({
                type: @type,
                port: @port,
                host: @host,
                threads: @threads
        })
        else
            -- set callback functions
            __leda.onThreadStarted = ->
                self\onThreadStarted(Thread!)
                
            __leda.onThreadStopped = ->
                self\onThreadStopped(Thread!)    
            
            __leda.onConnectionAccepted = ->
                self\onConnectionOpened(Connection!)
                
            __leda.onConnectionClosed = ->
                    self\onConnectionClosed(Connection!)    
                
            __leda.onConnectionDataReceived = ->
                self\onDataReceived(Connection!, __data)    
                

    -- method stubs                 
    
    onThreadStarted: (thread) =>
        
    onThreadStopped: (thread) =>
        
    onConnectionOpened: (connection) =>    
        @connections[connection\id!] = connection
        
    onConnectionClosed: (connection) =>    
        @connections[connection\id!] = nil
        
        
    onDataReceived: (connection, data) =>
        
    setTimer: (timeout, callback) =>
        __api.serverAddTimer(timeout, false, callback)    
        
    setTimeout: (timeout, callback) =>
            __api.serverAddTimer(timeout, true, callback)    
        
    send: (connection, data) =>
        connection\send(data)  
          
            
        
        
            
    
 



