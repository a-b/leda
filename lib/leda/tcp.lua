local common = require ('leda.common')

-- connection class
local Connection = class('Connection')

function Connection:initialize()
    self.connection = __leda.connection
end
     
Connection.address = function(self) return __api.serverConnectionGetAddress(self.connection) end
Connection.id = function(self) return __api.serverConnectionGetId(self.connection) end
Connection.send = function(self, data) return __api.serverConnectionSendData(self.connection, data) end

-- server class
local Server = class('Server', common.Server)

function Server:initialize(port, host)
    self.type = 'tcp'
    self.port = port or 12000
    self.host = host or 'localhost'
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
    
    common.Server.initialize(self)
end
        
return {Server = Server}