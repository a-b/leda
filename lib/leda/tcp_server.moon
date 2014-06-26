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
class TCPServer extends CommonServer
    --default type
    type: "tcp"
    -- connections map
    connections: {}
    
    -- constructor
    new: =>
        super()
        
        __leda.onConnectionAccepted = ->
            self\onConnectionOpened(Connection!)
            
        __leda.onConnectionClosed = ->
            self\onConnectionClosed(Connection!)    
            
        __leda.onConnectionDataReceived = ->
            self\onDataReceived(Connection!, __data)    
                

        
    -- new connection has been opened by server    
    onConnectionOpened: (connection) =>    
        @connections[connection\id!] = connection
        if @on_connection
            @on_connection(conection, true)
        
    -- connection    
    onConnectionClosed: (connection) =>    
        @connections[connection\id!] = nil
        if @on_connection
            @on_connection(conection, false)
        
    -- data was received on one of the connections
    onDataReceived: (connection, data) =>
        if @on_data
            @on_data(connection, data)
        
        
        
    -- send data on connection    
    send: (connection, data) =>
        connection\send(data)  
          
            
        
        
            
    
 



