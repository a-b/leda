export ^

require 'leda.common'
        
-- UDP server class
class UDPServer
    --default type
    type: "udp"
    -- default port
    port: 8000
    -- default host
    host: '127.0.0.1'
    -- default thread count
    threads: 1
    
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

            __leda.onUdpDataReceived = ->
                self\onDataReceived(__from, __data)    
                

    -- method stubs                 
    
    -- new thread was started
    onThreadStarted: (thread) =>
        
    -- thread was stopped 
    onThreadStopped: (thread) =>
                
    -- data was received from the address
    onDataReceived: (address, data) =>
    
    -- set timer. function specified in callback will be called every number of seconds specified by timeout    
    setTimer: (timeout, callback) =>
        __api.serverAddTimer(timeout, false, callback)    
    
    -- set timeout. function specified in callback will be called once the timeout passes     
    setTimeout: (timeout, callback) =>
            __api.serverAddTimer(timeout, true, callback)    
        
    -- send data on connection    
    send: (connection, data) =>
        connection\send(data)  
          
            
        
        
            
    
 



