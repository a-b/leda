export ^

require 'leda.common'
        
-- connection class
class Connection
    new: =>
        -- assigning via property results in @connection equal nil
        @connection = __connection
    
    address: => __api.serverConnectionGetAddress(@connection) 
    
    id: => __api.serverConnectionGetId(@connection) 
        
    send: (data) =>
        __api.serverConnectionSendData(@connection, data)
        
        
-- TCP server class
class TCPServer
    --default type
    type: "tcp"
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
    
    -- new thread was started
    onThreadStarted: (thread) =>
        
    -- thread was stopped 
    onThreadStopped: (thread) =>
        
    -- new connection has been opened by server    
    onConnectionOpened: (connection) =>    
        @connections[connection\id!] = connection
        
    -- connection    
    onConnectionClosed: (connection) =>    
        @connections[connection\id!] = nil
        
    -- data was received on one of the connections
    onDataReceived: (connection, data) =>
    
    -- set timer. function specified in callback will be called every number of seconds specified by timeout    
    setTimer: (timeout, callback) =>
        __api.serverAddTimer(timeout, false, callback)    
    
    -- set timeout. function specified in callback will be called once the timeout passes     
    setTimeout: (timeout, callback) =>
        __api.serverAddTimer(timeout, true, callback)    
        
    -- send data on connection    
    send: (connection, data) =>
        connection\send(data)  
          
            
        
        
            
    
 



