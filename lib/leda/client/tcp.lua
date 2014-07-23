--- Asynchronous TCP connection
-- @usage local tcp = require('leda.client.tcp')
-- local connection = tcp('localhost', 8080)
-- connection:send('test')
-- @module leda.client.tcp

local common = require 'leda.common'

local function getConnection(index)
    local connection = __leda.clientConnectionsMap[index or __leda.clientConnection]
    return connection
end

--- connection class
-- @type Connection
local Connection = class('Connection')

---  Create a TCP connection instance, Asynchronously tries to connect to address specified. Opened or error callbacks are invoked in case of success or failure
-- @param host address host
-- @param port address port
-- @param ssl ssl indicates whether to use ssl over the connection (leda has to  built with OpenSSL support), defaults to false
-- @return a new Connection
-- @usage local connection = client.Connection('some.domain', 8080)
-- @name Connection()
function Connection:initialize(host, port, ssl, callbacks)
    common.createClient()
    
    if not __leda.client then return end
    
    ssl = ssl or false
    self._open = false

    -- set callbacks
    __leda.onClientConnectionOpened = __leda.onClientConnectionOpened or function() 
        local connection = getConnection()
        connection._open = true
        if connection and  type(connection.opened) == 'function' then connection:opened() end
    end
    
    __leda.onClientConnectionClosed = __leda.onClientConnectionClosed or function() 
        local connection = getConnection()
        self:_closed()
        
        if connection and  type(connection.closed) == 'function' then connection:closed() end
    end
    
    __leda.onClientData = __leda.onClientData or function() 
        local connection = getConnection()
        if connection and  type(connection.data) == 'function' then connection:data(__leda.clientData) end
    end
    
    self.host = host
    self.port = port
    
    self.callbacks = callbacks or {}
    
    -- connect
    self:_connect(host, port, ssl)
    
    
end

--- send data over the connection
-- @param data data to send
function Connection:send(data)
    if not getConnection(self.__connection) then return end
    
    if self.__connection and self._open then __api.clientConnectionSendData(self.__connection, data) end
end

function Connection:_connect(host, port, ssl)  
    if __leda.init or not __leda.client then return end
    
    ssl = ssl or false
    local connection = __api.clientConnect(host, port, ssl)
    assert(connection, string.format("error creating connection to %s:%s", host, port))
    
    __leda.clientConnectionsMap = __leda.clientConnectionsMap or {}
    __leda.clientConnectionsMap[connection] = self
    
    self.__connection = connection
    
end

function Connection:_closed()
    self.__connection = nil

    __leda.clientConnectionsMap[__leda.clientConnection] = nil
    
    if not self._open then 
        self:_error(string.format("connection to %s:%s failed", self.host, self.port))
    end
    
    self._open = false
end


--- close the connection
function Connection:_error(message)
    
    if not self.error then self.error = self.callbacks.error end
    
    if type(self.error) == 'function' then self:error(message) end 
end


--- close the connection
function Connection:close()
    __api.clientConnectionClose(self.__connection)
end

--- opened callback. runs when the connection has been opened
-- @param callback function
-- @usage connection.opened = function(connection)
--    -- handle connection open
-- end
Connection.opened = nil

--- closed callback. runs when the connection has been closed
-- @param callback function
-- @usage connection.closed = function(connection)
--    -- handle connection close
-- end
Connection.closed = nil


--- error callback. runs when connection error occurs
-- @param callback function
-- @usage connection.error = function(connection, error)
--    print(string.format("connection error: %s", error))
-- end
Connection.error = nil

--- data callback. runs when new data has been received
-- @param callback function
-- @usage connection.data = function(connection, data)
--    -- handle received data
-- end
Connection.data = nil

return Connection