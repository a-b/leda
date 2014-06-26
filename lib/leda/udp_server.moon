export ^

require 'leda.common'
        
-- UDP server class
class UDPServer extends CommonServer
    type: "udp"                

    new: =>
        super()

        __leda.onUdpDataReceived = ->
            self\onDataReceived(__from, __data)

    -- data was received from the address
    onDataReceived: (address, data) =>
        if @on_data
            @on_data(address, data)
            
    -- send data on connection    
    sendTo: (ip, port, data) =>
        __api.serverSendTo(ip, port, data)
          
            
        
        
            
    
 



