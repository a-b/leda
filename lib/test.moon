class MyServer extends TCPServer
    test: =>
        print "test method override"
            
    onDataReceived: (connection, data) =>
        connection\send(data)
        
    onThreadStarted: (thread) =>
        callback = -> 
            for id, connection in pairs @connections
                connection\send("timer") 
            
        self\setTimer(1, callback)    
    
    test: =>
        print @threads    
        
        
        
with MyServer!    
--    \test!
    
    

  
  
  
  
  
  