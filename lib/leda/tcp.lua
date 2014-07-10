--- TCP server functionality
-- @usage local tcp = require('leda.tcp')
-- local server = tcp.Server(10000, 'localhost')
-- server.data = function(server, connection, data)
--  -- send back received data
--     connection:send(data)
-- end
--
-- @module leda.tcp

local common = require ('leda.common')

--- connection class
-- @type Connection
local Connection = class('Connection')

function Connection:initialize()
    self.connection = __leda.connection
end

--- get address of remote peer
-- @return address     
function Connection:address() 
    return __api.serverConnectionGetAddress(self.connection) 
end

--- get connection id
-- @return connection id
function Connection:id()
    return __api.serverConnectionGetId(self.connection) 
end

--- send data asynchronously over the connection
-- @param data data to send
function Connection:send(data) 
     __api.serverConnectionSendData(self.connection, data) 
end

--- server class
-- @type Server
local Server = class('Server', common.Server)

--- Create a server instance
-- @param port listening port
-- @param[opt] host listening host (defaults to 'localhost')
-- @return a new Server object
-- @usage local server = Server(12000, 'some.domain')
-- @name Server()
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

--- table of opened connections that maps connection ids to connection objects
Server.connections = {}

--- connection callback. runs whe connection to server is opened or closed
-- @param callback function
-- @usage server.connection = function(server, connection, opened)
--    local verb
--    if opened then verb = "opened" else verb = "closed" end
--    print(string.format("connection %s from %s was %s", connection:id(), connection:address(), verb))
-- end
Server.connection = nil

--- data callback. runs when new data has beem received by server
-- @param callback function
-- @usage server.data = function(server, connection, data)
--    print(string.format("received %s from %s", data, connection:address()))
-- end
Server.data = nil


--- thread callback. runs when thread has been started or stopped
-- @param callback callback function
-- @usage server.thread = function(server, thread, started) 
--      local verb = "started"
--      if stopped then verb = "stopped" end    
--      print(string.format("thread %s was %s", thread.id, verb))
--  end)
Server.thread = nil

--- set timeout. calls the function once after the seconds value elapses
-- @param seconds number of seconds 
-- @param callback function
function Server:timeout(...)
    common.Server.timeout(self, ...)
end    

--- set timer. calls the function every time the seconds value elapses
-- @param seconds number of seconds 
-- @param callback function
function Server:timer(...)
    common.Server.timer(self, ...)
end    

        
return {Server = Server}