--- HTTP server functionality
-- @usage local http = require('leda.server.http')
-- local server = http(8080, 'localhost')
-- server.request = function(server, response, request)
--     response.body = 'hello world!' 
-- end
--
-- @module leda.server.http

local common = require 'leda.common'
local util = require 'leda.util'

--- request class
-- @type Request
local Request = class('Request')

function Request:initialize()
    self._request = __leda.httpRequest
end

--- get request url
-- @return url
function Request:url()
     return __api.httpRequestGetUrl(self._request) 
end

--- get request method
-- @return method string
function Request:method()
     return __api.httpRequestGetMethod(self._request) 
end

--- get request body
-- @return body
function Request:body() 
    return __api.httpRequestGetBody(self._request) 
end

--- get request headers
-- @return table with request headers
function Request:headers()
     return  __api.httpRequestGetHeaders(self._request) 
end

--- get request remote peer address
-- @return address string 
function Request:address()
     return  __api.httpRequestGetAddress(self._request) 
end

--- request class
-- @type Response
local Response = class('Respose')

function Response:initialize()
    self.headers = {}
    self._response = __leda.httpResponse
end

--- send the response
function Response:send()
    __api.httpResponseSetStatus(self._response, self.status)
    __api.httpResponseSetHeaders(self._response, self.headers)
    __api.httpResponseSetBody(self._response, self.body)
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

return Server
