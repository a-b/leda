class MyServer extends Server
    test: =>
        print "test method override"
            
    onDataReceived: (connection, data) =>
        connection\send(data)
        
    onThreadStarted: (thread) =>
        callback = -> 
            print @connections
            for id, connection in pairs @connections
                connection\send("timer") 
            
        self\setTimer(1, callback)    
        
    
    test: =>
        print @threads    
        
        
        
with MyServer!    
    \test!
    
    

  
  
  
  
  
  