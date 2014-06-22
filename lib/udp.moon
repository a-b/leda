require 'leda.udp_server'

class MyServer extends UDPServer
    port: 12000

    onDataReceived: (address, data) =>
        print "received ", data, " from ", address

    onThreadStarted: (thread) =>
        print "started thread", thread.id
        
        callback = ->
            print "callback"
            
        self\setTimer(2, callback)    
        
        

MyServer!        

        --self\setTimeout(1, callback)

