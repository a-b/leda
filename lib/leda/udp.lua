local common = require ('leda.common')


local Server = class('Server', common.Server)

function Server:initialize(port, host)
    self.type = 'udp'
    self.port = port or 12000
    self.host = host or 'localhost'

    
    __leda.onUdpDataReceived = function()
        if type(self.data) == 'function' then
            self:data(__leda.from, __leda.data)
        end
    end
    
    common.Server.initialize(self)
end

local function send(host, port, data)
    -- todo: implement
end
        
return {Server = Server}