local luaunit = require 'luaunit'
local http = require 'leda.http'
local client = require 'leda.client'

TestHttp = {}

local host = 'localhost'

math.randomseed(os.time())
local port = 10000 + math.random(1000)

local TestHttpLoad = class('TestHttpLoad')

function TestHttpLoad:initialize(url, requestsCount)        
    self.connections = {}
    self.requests = 0
    self.responses = 0
    self.requestsCount = requestsCount

    -- create needed mumber of connections
    for i = 1, 10 do
        local connection = client.HttpConnection(url)
        
        connection.error = function()
            self.errors = self.errors + 1
            connection:connect()
        end
        
        table.insert(self.connections, connection) 
     end
     
    self.start = os.time()
    
    self:_sendRequests()
end


function TestHttpLoad:_sendRequests()
    if self.requests > self.requestsCount then return end
        
    for i, connection in ipairs(self.connections) do
        connection:get(function(response)
            self.responses = self.responses + 1
        end)
        
        
        self.requests = self.requests + 1
    end
    
    -- schedule recursive call in the next run of the event loop
    client.timeout(0, function()
        self:_sendRequests()
        end)
end

local server = http.Server(port, host)

server.request = function(server, request, response)
    response.body = 'test'
end

-- send 10000 requests to server
local load = TestHttpLoad(string.format("%s:%s", host, port), 10000)

client.timeout(3, function()
        os.exit(luaunit:run())
        end)
    
function  TestHttp:testRun()
    assert(load.requests == load.responses)
end
--



