local common = require 'leda.common'
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

return {Server = Server}