-- UDP server example
require 'leda.udp_server'

class ExampleServer extends UDPServer
    port: 9600
    threads: 1
    
    onThreadStarted: (thread) =>
        print "started thread ", thread.id
        
    onDataReceived: (address, data) =>    
        print string.format("received data %s  from %s",data, address)
        
ExampleServer!        