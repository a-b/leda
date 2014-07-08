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

function Connection:initialize(host, port, ssl)
    createClient()
    ssl = ssl or false
    
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
    
    self:_connect(host, port, ssl)
end

function Connection:send(data)
    if not getConnection(self.__connection) then return end
    
    if self.__connection then __api.clientConnectionSendData(self.__connection, data) end
end

function Connection:_connect(host, port, ssl) 
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
    
    
    self._open = false
    
    if not self._open then 
        if type(self.error) == 'function' then self:error(string.format("connection to %s:%s failed", self.host, self.port)) end 
    end
end

function Connection:close()
    __api.clientConnectionClose(self.__connection)
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

-- http connection class
local HttpConnection = class('HttpConnection', Connection)

function HttpConnection:initialize(url)
    self.url = utility.parseUrl(url)    
    
    if self.url.scheme and not self.url.host then 
        self.url.host = self.url.scheme 
        self.url.scheme = nil
    end
    
    local ports = {https=443, http=80}
    
    if self.url.scheme then
        self.type = self.url.scheme
    else
        self.type = 'http'
    end
    
    if tonumber(self.url.path) then
         self.url.port = self.url.path 
         self.url.path = nil
     end
     
    self.url.port = self.url.port or ports[self.type]
    self.url.path = self.url.path or '/'
    
    for _, method in ipairs{'POST', 'GET', 'PUT', 'DELETE'} do
        self[method:lower()] = function(self, path, headers, body, callback)
            self:_prepareRequest(method, path, headers, body, callback)
        end
    end
    
    self.reconnects = 0
    
    Connection.initialize(self, self.url.host, self.url.port, self.type == 'https')
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
    self._response = {headers = {}, data = "", body = ""}
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
                    self._response.headers[parts[1]:lower()] = parts[2]:trim()
                end
            end
        end
    end
    
    if self._response.header then
        if self._response.headers['content-length'] then
            self._response.length = tonumber(self._response.headers['content-length'])
            
        elseif self._response.headers['transfer-encoding'] == 'chunked'  then
            self._response.chunked = true
            
            if not self._response.length then
                local n = self._response.data:find("\n")
                if n then 
                    local boundary = self._response.data:sub(1, n) 
                    self._response.length = tonumber(boundary:trim("\n"), 16)
                    print(string.format("chunk length %s", self._response.length))
                    self._response.data = self._response.data:sub(n + 3)
                else
                    error("unable to parse http response")
                end
            end
        else
            error("unable to parse http response")
        end
    else
        return
    end
    
        
    local next
    if self._response.length == 0 then
        -- remove last line from response
        self._response.data = self._response.data:sub(2)
        self._response.full = true
        next = true
    else
        if #self._response.data >= self._response.length then
            self._response.body = self._response.body .. self._response.data:sub(1, self._response.length)
            self._response.data = self._response.data:sub(self._response.length + 1)
            self._response.full = not self._response.chunked
            self._response.length = nil
            next = true
        end
    end
    
    
    if self._response.full then
        local data = self._response.data or ''
        self._response.data = nil
        -- response callback
        if type(self.responseCallback) == 'function' then self.responseCallback(self._response) end    
        self:_newResponse()
        self._response.data = data
    end
            
    -- if more responses left call self
    if #self._response.data > 0 and next then self:data() end
end

function HttpConnection:connect()
    -- reconnect
   self:_connect(self.url.host, self.url.port, self.type == 'https')
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