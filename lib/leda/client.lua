--- High perfomance asynchronous HTTP and TCP client functionality
-- @module leda.client

local utility = require 'leda.utility'
local common = require 'leda.common'

local function createClient(threads)
    if __leda.init then
        __leda.client =__leda.client or __api.clientCreate(threads or 1)
    end
end


local function threadCount()
    return __leda.threadCount
end


local client = {}

-- utility functions

local addTimer = function(timeout, once, callback) 
    createClient()
    
    if not __leda.init and __leda.client then __api.clientAddTimer(timeout, once, callback) end
end

local function getConnection(index)
    local connection = __leda.clientConnectionsMap[index or __leda.clientConnection]
    return connection
end



--- set client threads
-- @param count client thread count (defaults to 1)
function client.setThreads(count)
    createClient(count)
end

--- get current thread
-- @return current thread
function client.currentThread()
    return common.Thread()
end

--- set timer. calls the function every time the seconds value elapses
-- @param seconds number of seconds 
-- @param callback function
client.timer = function(timeout, callback)    
    addTimer(timeout, false, callback)
end

--- set timeout. calls the function once after the seconds value elapses
-- @param seconds number of seconds 
-- @param callback function
client.timeout = function(timeout, callback) 
    addTimer(timeout, true, callback)
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
function Connection:initialize(host, port, ssl)
    createClient()
    
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


client.Connection = Connection

--- http connection class
--- @type HttpConnection
local HttpConnection = class('HttpConnection', Connection)

---  Create a HTTP connection. Asynchronously tries to connect to url specified. Opened or error callbacks are invoked in case of success or failure
-- @param url url to connect to 
-- @param[opt] errorCallback error callback function
-- @return a new HttpConnection
-- @usage local connection = client.HttpConnection('www.google.com')
-- connection.get(function(response) print(response.body) end)
-- @name HttpConnection()
function HttpConnection:initialize(url, errorCallback)
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

    Connection.initialize(self, self.url.host, self.url.port, self.type == 'https')
end

--- perform a GET request asynchronously. Function can receive callback function as any argument
-- @param path request path or callback
-- @param[opt] headers additional headers to send with request
-- @param[opt] callback callback function that is run when the response has been received. It receives a table with fields: headers, status, body
-- @usage connection:get(function(response)
--  print(response.body)
--      end)
function HttpConnection:get(path, headers, callback)    
end

--- perform a POST request asynchronously. Function can receive callback function as any argument
-- @param path request path 
-- @param headers additional headers to send with request
-- @param body additional headers to send with request
-- @param callback callback function that is run when the response has been received. It receives a table with fields: headers, status, body
-- @usage local headers = {}
--      headers['content-type'] = 'application/x-www-form-urlencoded'
--      connection:post('/form', headers, "name=value", function(response)
--      print(response.body)
--    end)
function HttpConnection:post(path, headers, body, callback)    
end

--- perform a PUT request asynchronously. Function can receive callback function as any argument
-- @param path request path or callback
-- @param[opt] headers additional headers to send with request
-- @param[opt] body request body
-- @param[opt] callback callback function that is run when the response has been received. It receives a table with fields: headers, status, body
-- @usage connection:put('/path', function(response)
--  print(response.body)
--      end)
function HttpConnection:put(path, headers, body, callback)    
end

--- perform a DELETE request asynchronously. Function can receive callback function as any argument
-- @param path request path or callback
-- @param[opt] headers additional headers to send with request
-- @param[opt] callback callback function that is run when the response has been received. It receives a table with fields: headers, status, body
-- @usage connection:delete('/path', function(response)
--  print(response.body)
--      end)
function HttpConnection:delete(path, headers, callback)    
end

function HttpConnection:opened()
    if self._request then  self:_send()  end
end

function HttpConnection:_send()
    if self._request and self._open then
        Connection.send(self, self._request)
        self._request = nil
    end
end

HttpConnection.Parser = class('HttpConnection.Parser')

function HttpConnection.Parser:initialize(connection)
    self.connection = connection
    self.data = ''
end

function HttpConnection.Parser:_newResponse()
    return {headers = {},  body = ""}
end

function HttpConnection.Parser:_error(...)
    self.connection:_error()
end

function HttpConnection.Parser:add(data)
    self.response = self.response or self._newResponse()
    
    if data then
        self.data = self.data .. data
    end
    
    -- parse header
    if not self.header then 
        -- parse headers
        local s, e = self.data:find('\r\n\r\n') 
        if s then
            self.header = self.data:sub(1, s)
            self.data = self.data:sub(e + 1)
            for i, line in ipairs(self.header:split('\r\n'))  do
                if i == 1 then
                    -- status line
                    local parts = line:split(' ')
                    self.response.status = tonumber(parts[2])
                    self.response.reason = tonumber(parts[3])
                else
                    -- header line
                    local colon = line:find(':')
                    if not colon then
                        self:_error("error parsing response")
                        return
                    end
                    
                    local name = line:sub(1, colon - 1)
                    local value = line:sub(colon + 2)
                    self.response.headers[name:lower()] = value:trim()
                end
            end
        end
    end
    
    if self.header then
        if self.response.headers['content-length'] then
            self.length = tonumber(self.response.headers['content-length'])
            
        elseif self.response.headers['transfer-encoding'] == 'chunked'  then
            self.chunked = true
            
            if not self.length then
                local n = self.data:find("\n")
                if n then 
                    local boundary = self.data:sub(1, n) 
                    self.length = tonumber(boundary:trim("\n"), 16)
                    self.data = self.data:sub(n + 3)
                end
            end
        end
    else
        return
    end
    
    if not self.length then 
        self:_error("error parsing response")
        return
    end
    
    local next
    if self.length == 0 then
        -- remove last line from response
        self.data = self.data:sub(2)
        self.full = true
        next = true
    else
        if #self.data >= self.length then
            self.response.body = self.response.body .. self.data:sub(1, self.length)
            self.data = self.data:sub(self.length + 1) or ''
            self.full = not self.chunked
            self.length = nil
            next = true
        end
    end
    
    
    if self.full then
        -- response callback
        if type(self.connection.responseCallback) == 'function' then self.connection.responseCallback(self.response) end    
        self.response = self:_newResponse()
        self.full = nil
        self.header  = nil
    end
            
    -- if more responses left call self
    if #self.data > 0 and next then self:add() end
end

-- connection data 
function HttpConnection:data(data)
    self.parser:add(data)
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
    self.parser = self.parser or HttpConnection.Parser(self)
    
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


--- close the connection
function HttpConnection:close()
    Connection.close(self)
end

--- error callback. runs when connection error occurs
-- @param callback function
-- @usage connection.error = function(connection, error)
--    print(error)
-- end
HttpConnection.error = nil

client.HttpConnection = HttpConnection

return client