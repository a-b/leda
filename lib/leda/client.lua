local common = require 'leda.common'

-- utility functions
local function getConnection(index)
    local connection = __leda.clientConnectionsMap[index or __leda.clientConnection]
    return connection
end

local function createClient(threads)

    if __leda.init then
        __leda.client =__leda.client or __api.clientCreate(threads or 1)
    end
end




local function setThreads(count)
    createClient(count)    
    
end

local function currentThread()
    return common.Thread()
end

local function threadCount()
    return __leda.threadCount
end

-- connection class
local Connection = class('Connection')

function Connection:initialize(host, port)
    createClient()
    
    self._open = false
    
    if __leda.init then return end
    
    __leda.onClientConnectionOpened = onClientConnectionOpened or function() 
        local connection = getConnection()
        connection._open = true
        if connection and  type(connection.opened) == 'function' then connection:opened() end
    end
    
    __leda.onClientConnectionClosed = onClientConnectionClosed or function() 
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
    
    self:_connect(host, port)
end

function Connection:send(data)
    if not getConnection(self.__connection) then return end
    
    if self.__connection then __api.clientConnectionSendData(self.__connection, data) end
end

function Connection:_connect(host, port) 
    local connection = __api.clientConnect(host, port)
    assert(connection, "error connecting to %s:%s", host, port)
    
    __leda.clientConnectionsMap = __leda.clientConnectionsMap or {}
    __leda.clientConnectionsMap[connection] = self
    
    self.__connection = connection
    
end

function Connection:_closed()
    self.__connection = nil

    __leda.clientConnectionsMap[__leda.clientConnection] = nil
        
    if not self._opened then    
        assert(self._open, string.format("connection to %s:%s failed" , self.host, self.port))
    end
    
    self._open = false
    self._opened = true
end

function Connection:close()
    __api.clientConnectionClose(self.__connection)
    self:_closed()
end

local addTimer = function(timeout, once, callback) 
    createClient()
    if not __leda.init then __api.clientAddTimer(timeout, once, callback) end
end

local timer = function(timeout, callback)    
    addTimer(timeout, false, callback)
end
    
local timeout = function(timeout, callback) 
    addTimer(timeout, true, callback)
end

return {Connection = Connection, timer = timer, timeout = timeout, currentThread = currentThread, threadCount = threadCount, setThreads = setThreads}