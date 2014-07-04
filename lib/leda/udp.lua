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
        if type(self.data) == 'function' then
            self:data(__leda.from, __leda.data)
        end
    end
    
    common.Server.create(self)
end
        
return {Server = Server}