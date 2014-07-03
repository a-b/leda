local common = require ('leda.common')


local Server = {__index=Server}
setmetatable(Server,
     {
         __index = common.Server,
         __call = function(cls, ...)
             cls:create(...)
             return cls
        end
     })
    
function Server:create(port, host)
    self.type = 'udp'
    self.port = port or 12000
    self.host = host or ''

    
    __leda.onUdpDataReceived = function()
        if type(self._onDataReceived) == 'function' then
            self._onDataReceived(__leda.from, __leda.data)
        end
    end
    
    common.Server.create(self)
end
        
function Server:connection(callback) 
    self._onConnectionCallback = callback 
end

function Server:data(callback) 
    self._onDataReceived = callback
end

return {Server = Server}