--- HTTP server functionality
-- @usage local http = require('leda.http')
-- local server = http.Server(8080, 'localhost')
-- server.request = function(server, response, request)
--     response.body = 'hello world!' 
-- end
--
-- @module leda.http

local common = require 'leda.common'
local util = require 'leda.util'

--- request class
-- @type Request
local Request = class('Request')

--- get request url
-- @return url
function Request:url()
     return __api.httpRequestGetUrl(__leda.httpRequest) 
end

--- get request method
-- @return method string
function Request:method()
     return __api.httpRequestGetMethod(__leda.httpRequest) 
end

--- get request body
-- @return body
function Request:body() 
    return __api.httpRequestGetBody(__leda.httpRequest) 
end

--- get request headers
-- @return table with request headers
function Request:headers()
     return  __api.httpRequestGetHeaders(__leda.httpRequest) 
end

--- get request remote peer address
-- @return address string 
function Request:address()
     return  __api.httpRequestGetAddress(__leda.httpRequest) 
end

--- request class
-- @type Response
local Response = class('Respose')

function Response:initialize()
    self.headers = {}
end

function Response:send()
    __api.httpResponseSetStatus(__leda.httpResponse, self.status)
    __api.httpResponseSetHeaders(__leda.httpResponse, self.headers)
    __api.httpResponseSetBody(__leda.httpResponse, self.body)
end

--- response headers table
Response.headers = {}

--- response status field
Response.status = 200

-- response body
Response.body = ''

--- server class
-- @type Server
local Server = class('Server', common.Server)

--- Create a server instance
-- @param port listening port
-- @param[opt] host listening host (defaults to 'localhost')
-- @return a new Server object
-- @usage local server = Server(8080, 'some.domain')
-- @name Server()
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
            response.headers['Content-Type'] = 'text/plain'
        end

        response.headers['Date'] = util.formatTime(os.time())
        
        -- send response
        response:send()
    end
    
    
    -- call parent constructor
    common.Server.initialize(self)
end

--- set request callback
-- @param callback callback function.
-- @usage server.request = function(server, request, response) 
-- -- handle request
--  end)
Server.request = nil

--- thread callback. runs when thread has been started or stopped
-- @param callback callback function
-- @usage server.thread = function(server, thread, started) 
--      local verb = "started"
--      if stopped then verb = "stopped" end    
--      print(string.format("thread %s was %s", thread.id, verb))
--  end)
Server.thread = nil


return {Server = Server}
