--- Asynchronous HTTP client
-- @usage local http = require('leda.client.http')
-- http('www.google.com'):get('/', function(response)
--  print(response.body) 
--end)

-- @module leda.client.http

local tcp = require 'leda.client.tcp'
local util = require 'leda.util'

--- http connection class
--- @type Connection

local Connection = class('Connection', tcp)

---  Create HTTP connection. Asynchronously tries to connect to url specified. Opened or error callbacks are invoked in case of success or failure
-- @param url url to connect to 
-- @param[opt] errorCallback error callback function
-- @return a new Connection
-- @usage local connection = client.Connection('www.google.com')
-- connection.get(function(response) print(response.body) end)
-- @name Connection()
function Connection:initialize(url, errorCallback)
    self.url = util.parseUrl(url)   
    self.type = self.url.scheme 
    self.url.path = self.url.path or '/'
    
    for _, method in ipairs{'POST', 'GET', 'PUT', 'DELETE'} do
        self[method:lower()] = function(self, path, headers, body, callback)
            self:_prepareRequest(method, path, headers, body, callback)
        end
    end
    
    self.version = string.format('leda.client.Connection/%s', __api.getVersion())

    tcp.initialize(self, self.url.host, self.url.port, self.type == 'https', {error = errorCallback})
end


--- perform a GET request asynchronously. Function can receive callback function as any argument
-- @param path request path or callback
-- @param[opt] headers additional headers to send with request
-- @param[opt] callback callback function that is run when the response has been received. It receives a table with fields: headers, status, body
-- @usage connection:get(function(response)
--  print(response.body)
--      end)
function Connection:get(path, headers, callback)    
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
function Connection:post(path, headers, body, callback)    
end

--- perform a PUT request asynchronously. Function can receive callback function as any argument
-- @param path request path or callback
-- @param[opt] headers additional headers to send with request
-- @param[opt] body request body
-- @param[opt] callback callback function that is run when the response has been received. It receives a table with fields: headers, status, body
-- @usage connection:put('/path', function(response)
--  print(response.body)
--      end)
function Connection:put(path, headers, body, callback)    
end

--- perform a DELETE request asynchronously. Function can receive callback function as any argument
-- @param path request path or callback
-- @param[opt] headers additional headers to send with request
-- @param[opt] callback callback function that is run when the response has been received. It receives a table with fields: headers, status, body
-- @usage connection:delete('/path', function(response)
--  print(response.body)
--      end)
function Connection:delete(path, headers, callback)    
end

function Connection:opened()
    if self._request then  self:_send()  end
end

function Connection:_send()
    if self._request and self._open then
        Connection.send(self, self._request)
        self._request = nil
    end
end

Connection.Parser = class('Connection.Parser')

function Connection.Parser:initialize(connection)
    self.connection = connection
    self.data = ''
end

function Connection.Parser:_newResponse()
    return {headers = {},  body = ""}
end

function Connection.Parser:_error(...)
    self.connection:_error()
end

function Connection.Parser:add(data)
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
        self.chunked = false
        self.length = nil
    end
            
    -- if more responses left call self
    if #self.data > 0 and next then self:add() end
end

-- connection data 
function Connection:data(data)
    self.parser:add(data)
end
    
function Connection:_prepareRequest(method, path, headers, body, callback)
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
    self.parser = self.parser or Connection.Parser(self)
    
    local request = string.format("%s %s HTTP/1.1\r\n", method, path)
    headers = headers or {}

    headers['User-Agent'] = self.version 
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
function Connection:close()
    Connection.close(self)
end


--- error callback. runs when connection error occurs
-- @param callback function
-- @usage connection.error = function(connection, error)
--    print(error)
-- end
Connection.error = nil

return Connection
