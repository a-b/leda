local common = require 'leda.common'
local utility = require 'leda.utility'

-- request class
local Request = class('Request')

Request.url = function() print("url") return __api.httpRequestGetUrl(__leda.httpRequest) end
Request.method = function() return __api.httpRequestGetMethod(__leda.httpRequest) end
Request.body = function() return __api.httpRequestGetMethod(__leda.httpRequest) end
Request.headers = function() return  __api.httpRequestGetHeaders(__leda.httpRequest) end
Request.address = function() return  __api.httpRequestGetAddress(__leda.httpRequest) end

-- response class
local Response = class('Respose')

function Response:initialize()
    self.headers = {}
    self.status = 200
    self.body = ''
end
    
function Response:send()
    __api.httpResponseSetStatus(__leda.httpResponse, self.status)
    __api.httpResponseSetHeaders(__leda.httpResponse, self.headers)
    __api.httpResponseSetBody(__leda.httpResponse, self.body)
end

-- Server class
local Server = class('Server', common.Server)
    
function Server:initialize(port, host)
    self.type = 'http'
    self.port = port or 8080
    self.host = host or 'localhost'
    
    -- set callbacks
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

        response.headers['Date'] = utility.formatTime(os.time())
        
        -- send response
        response:send()
    end
    
    
    -- call parent constructor
    common.Server.initialize(self)
end


return {Server = Server}
