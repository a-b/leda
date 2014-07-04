local function getConnection()
    local connection = __leda.clientConnectionsMap[__leda.clientConnection]
    if connection then connection.__connection = __leda.clientConnection end
        
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

local Connection = {__index=Connection}

setmetatable(Connection, {__call = function(cls, ...)
     cls:create(...)
     return cls 
     end
})



function Connection:create(host, port)
    createClient()
    
    if __leda.init then return end
    
    __leda.onClientConnectionOpened = onClientConnectionOpened or function() 
        local connection = getConnection()
        if connection and  type(connection.opened) == 'function' then connection:opened() end
    end
    
    __leda.onClientConnectionClosed = onClientConnectionClosed or function() 
        local connection = getConnection()
        if connection and  type(connection.closed) == 'function' then connection:closed() end
    end
    
    __leda.onClientData = __leda.onClientData or function() 
        local connection = getConnection()
        if connection and  type(connection.data) == 'function' then connection:data(__leda.clientData) end
    end
    
    local connection = __api.clientConnect(host, port)
    __leda.clientConnectionsMap = __leda.clientConnectionsMap or {}
    
    __leda.clientConnectionsMap[connection] = self
end


function Connection:send(data)
    __api.clientConnectionSendData(self.__connection, data)
end

function Connection:close()
    __api.clientConnectionClose(self.__connection)
    __leda.clientConnectionsMap[__leda.clientConnection] = nil
    self.__connection = nil
    __leda.clientConnection = nil
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

return {Connection = Connection, timer = timer, timeout = timeout}

--
-- Connection()
-- connection.open = function()
-- end
--
-- connection.data = function()
-- end
--
