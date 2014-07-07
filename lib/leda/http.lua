local common = require 'leda.common'
local client = require 'leda.client'


local Request = {__index=Request}

setmetatable(Request, {__call = function(cls, ...) return cls end})

Request.url = function() return __api.httpRequestGetUrl(__leda.httpRequest) end
Request.method = function() return __api.httpRequestGetMethod(__leda.httpRequest) end
Request.body = function() return __api.httpRequestGetMethod(__leda.httpRequest) end
Request.headers = function() return  __api.httpRequestGetHeaders(__leda.httpRequest) end
Request.address = function() return  __api.httpRequestGetAddress(__leda.httpRequest) end


local Response = {
    headers = {},
    status = 200,
    body = '',
    __index = Response
}

setmetatable(Response, {__call = function(cls, ...) return cls.create(cls, ...) end})

Response.create = function(self)
    self.headers = {}
    return self
end
    
Response.send = function(self)
    __api.httpResponseSetStatus(__leda.httpResponse, self.status)
    __api.httpResponseSetHeaders(__leda.httpResponse, self.headers)
    __api.httpResponseSetBody(__leda.httpResponse, self.body)
end

-- Server class
local Server = {__index=Server}
setmetatable(Server, {__index = common.Server, __call = function(cls, ...) cls:create(...) return cls end})
    
function Server:create(port, host)
    self.type = 'http'
    self.port = port or 8080
    self.host = host or ''
    
    __leda.onHttpRequest = function()
        local response = Response()
        
        if type(self.request) == 'function' then self:request(Request(), response) end
        
        -- set some headers and serialize tables to json
        if not response.headers['Content-Type'] then
            local contentType
            if type(response.body) == 'table' then
                contentType = 'application/json'
                response.body = json.encode(response.body) 
            else
                contentType = 'text/plain'
            end

            response.headers['Content-Type'] = contentType
        end

        response.headers['Date'] = common.formatTime(os.time())
        
        response:send()
    end
    
    common.Server.create(self)
end

-- http Client class
local Client = class('Client', client.Connection)


for _, method in ipairs{'POST', 'GET', 'PUT', 'DELETE'} do
    Client[method:lower()] = function(client, path, headers, body, callback)
        Client._prepareRequest(client, method, path, headers, body, callback)
    end
end


function Client:initialize(url)
    self.url = common.parseUrl(url)    
    
    
    local ports = {https=443, http=80}
    if self.url.scheme then
        self.type = self.url.scheme
    else
        self.type = 'http'
    end
    
    self.url.port = self.url.port or ports[self.type]
    self.url.path = self.url.path or '/'

    client.Connection.initialize(self, self.url.host, self.url.port)
    self.reconnects = 0
    
    self._response = {headers = {}, data = ""}
end

function Client:opened()
    if self._request then self:send() self:send()  end
end

function Client:send()
    if self._request and self.__connection then
        client.Connection.send(self, self._request)
        self._request = nil
    end
end

function Client:data(data)
    if data then
        self._response.data = self._response.data .. data
    end
    
    if not self._response.header then 
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
        
        if type(self.responseCallback) == 'function' then self.responseCallback{headers = self._response.headers, body = body, status =  self._response.status} end
        self._response.header = nil
        self._response.headers = {}
        
        self._response.data = self._response.data:sub(contentLength + 1) or ""
        
        if #self._response.data > 0 then self:data() end
    end
end

function Client:closed()
    -- reconnect
   self:_connect(self.url.host, self.url.port)
   self.reconnects = self.reconnects + 1
end
    

function Client:_prepareRequest(method, path, headers, body, callback)
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
    headers['User-Agent'] = 'leda.http.Client/0.2'
    headers['Host'] = string.format("%s:%d", self.url.host, self.url.port)
    headers['Accept'] = "*/*"
    headers['Connection'] = "keep-alive"
    
    if body then
       headers['Content-Length'] = #body
    end
    
    for key, value in pairs(headers) do
        request = request .. string.format("%s: %s\r\n", key, value)
    end
    
    request = request .. "\r\n"
    
    if body then request = request .. body end
    self._request = request
    
    
    if self.__connection and self._open then self:send(request) end
end

return {Server = Server, Client=Client}
