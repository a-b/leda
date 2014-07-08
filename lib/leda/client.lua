local utility = require 'leda.utility'
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

-- http Client class
local HttpConnection = class('HttpConnection', Connection)

function HttpConnection:initialize(url)
    self.url = utility.parseUrl(url)    
    
    
    local ports = {https=443, http=80}
    if self.url.scheme then
        self.type = self.url.scheme
    else
        self.type = 'http'
    end
    
    self.url.port = self.url.port or ports[self.type]
    self.url.path = self.url.path or '/'
    
    for _, method in ipairs{'POST', 'GET', 'PUT', 'DELETE'} do
        self[method:lower()] = function(self, path, headers, body, callback)
            self:_prepareRequest(method, path, headers, body, callback)
        end
    end
    
    self.reconnects = 0
    
    Connection.initialize(self, self.url.host, self.url.port)
end

function HttpConnection:opened()
    if self._request then  self:_send()  end
end

function HttpConnection:_send()
    if self._request and self.__connection and self._open then
        Connection.send(self, self._request)
        self._request = nil
    end
end

function HttpConnection:_newResponse()
    self._response = {headers = {}, data = ""}
end

-- connection data 
function HttpConnection:data(data)
    if not self._response then self:_newResponse() end
    
    if data then
        self._response.data = self._response.data .. data
    end
    
    -- parse http response
    if not self._response.header then 
        -- parse headers
        local s, e = self._response.data:find('\r\n\r\n') 
        if s then
            self._response.header = self._response.data:sub(1, s)
            self._response.data = self._response.data:sub(e + 1)
            for i, line in ipairs(self._response.header:split('\r\n'))  do
                if i == 1 then
                    -- status line
                    local parts = line:split(' ')
                    self._response.status = tonumber(parts[2])
                    self._response.reason = tonumber(parts[3])
                else
                    -- header line
                    local parts = line:split(': ')
                    self._response.headers[parts[1]:lower()] = parts[2]
                end
            end
        end
    end
    
    if self._response.header then
        if not self._response.headers['content-length'] then error("content-length header not found") end
    else
        return
    end
    
    local contentLength = tonumber(self._response.headers['content-length'])
    
    if #self._response.data >= contentLength then
        local body = self._response.data:sub(1, contentLength)
        
        -- response callback
        if type(self.responseCallback) == 'function' then self.responseCallback{headers = self._response.headers, body = body, status =  self._response.status} end

        local data = self._response.data:sub(contentLength + 1) or ""
        self:_newResponse()
        self._response.data = data
        
        -- if more responses left call self
        if #self._response.data > 0 then self:data() end
    end
end

function HttpConnection:closed()
    -- reconnect
   self:_connect(self.url.host, self.url.port)
   self.reconnects = self.reconnects + 1
end
    

function HttpConnection:_prepareRequest(method, path, headers, body, callback)
    if type(path) == 'function' then 
        self.responseCallback = path 
        path = self.url.path 
    end
    
    if type(headers) == 'function' then 
        self.responseCallback = headers
        headers = {}
    end
    
    if type(body) == 'function' then
         self.responseCallback = body 
         body = nil
    end
    
    if type(callback) == 'function' then self.responseCallback = callback end
    
    local request = string.format("%s %s HTTP/1.1\r\n", method, path)
    headers = headers or {}
    headers['User-Agent'] = 'leda.client.HttpConnection/0.2'
    headers['Host'] = string.format("%s:%d", self.url.host, self.url.port)
    headers['Accept'] = "*/*"
    
    if body then
       headers['Content-Length'] = #body
    end
    
    for key, value in pairs(headers) do
        request = request .. string.format("%s: %s\r\n", key, value)
    end
    
    request = request .. "\r\n"
    
    if body then request = request .. body end
    self._request = request
    
    
    self:_send()
end

return {Connection = Connection, timer = timer, timeout = timeout, currentThread = currentThread, threadCount = threadCount, setThreads = setThreads, HttpConnection = HttpConnection}