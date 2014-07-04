local common = require ('leda.common')

local Connection = {__index=Connection}

setmetatable(Connection, {__call = function(cls, ...)
     cls:create(...)
     return cls 
     end
     })

function Connection:create()
    self.connection = __leda.connection
end
     
     

Connection.address = function(self) return __api.serverConnectionGetAddress(self.connection) end
Connection.id = function(self) return __api.serverConnectionGetId(self.connection) end
Connection.send = function(self, data) return __api.serverConnectionSendData(self.connection, data) end


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
    self.type = 'tcp'
    self.port = port or 12000
    self.host = host or ''
    self.connections = {}

    __leda.onConnectionAccepted = function()
        local connection = Connection()
        if type(self.connection) == 'function' then
            self:connection(connection, true)
        end
        self.connections[connection:id()] = connection
    end
    
    __leda.onConnectionClosed = function()
        local connection = Connection()
        if type(self.connection) == 'function' then
            self:connection(connection, false)
        end
        self.connections[connection:id()] = nil
    end
    
    __leda.onConnectionDataReceived = function()
        if type(self.data) == 'function' then
            self:data(Connection(), __leda.data)
        end
    end
    
    common.Server.create(self)
end
        

return {Server = Server}