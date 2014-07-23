local luaunit = require 'luaunit'

local http = require 'leda.client.http'
local client = require 'leda.client'

TestHttp = {}

local host = 'localhost'

math.randomseed(os.time())
local port = 10000 + math.random(2000) + math.random(1000)

local TestHttpLoad = class('TestHttpLoad')

function TestHttpLoad:initialize(url, requestsCount)        
    self.connections = {}
    self.requests = 0
    self.responses = 0
    self.requestsCount = requestsCount
    self.errors = 0
    self.url = url

    -- create needed mumber of connections
    for i = 1, 10 do
        self:_newConnection(i)
     end
     
    self:_sendRequests()
end

function TestHttpLoad._connectionError(connection, error)
    connection.load.errors = connection.load.errors + 1
    -- establish new connection
    connection.load:_newConnection(connection.id)
end


function TestHttpLoad:_newConnection(id)
    
    local connection = http(self.url, {error = TestHttpLoad._connectionError})
    connection.id = id
    connection.load = self
    
    self.connections[id] = connection    
    
    return connection
end


function TestHttpLoad:_sendRequests()   
    if self.requests >= self.requestsCount then return end
    
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


local server = require 'leda.server.http'

server = server(port, host)
local testHeaderName = 'x-leda-test'
local testHeaderValue = 'test'
local testBody = "test"
local path1 = "/path1"
local path2 = "/path2"



server.request = function(server, request, response)
    local url = request:url()

    if url  == '/' then
        response.body = 'test'
    end
        
    if #url > 2 then
        local header = request:headers()[testHeaderName]
        response.body = {url = request:url(), method=request:method(), body=request:body(), header = header}
        response.headers[testHeaderName] = testHeaderValue
    end
    
    response:send()
end

local url = string.format("%s:%s", host, port)
-- send 10000 requests to server

local load = TestHttpLoad(url, 10000)

client.timeout(1, function()
    local connection = http(url)
    local headers = {}
    headers[testHeaderName] = testHeaderValue
    connection:get(path1, headers, function(response) 
        TestHttp.responseGet = response
    end)
end)

client.timeout(1, function()
    local connection = http(url)
    local headers = {}
    headers[testHeaderName] = testHeaderValue
    connection:post(path2, headers, testBody, function(response) 
        TestHttp.responsePost = response
    end)
end)
    
    

client.timeout(10, function()
        os.exit(luaunit:run())
        end)
    
function  TestHttp:testRun()
            print(load.requests, load.responses)

            assert(load.requests == load.responses)


if json then
      local body = json.decode(TestHttp.responseGet.body) 
    assert(body.url == path1)
    assert(body.header == testHeaderValue)
    assert(body.method == 'GET')
          assert(TestHttp.responseGet.headers['content-type'] == 'application/json')
  
   assert(TestHttp.responsePost.headers['content-type'] == 'application/json')
   
    body = json.decode(TestHttp.responsePost.body) 
    assert(body.url == path2)
    assert(body.header == testHeaderValue)
    assert(body.method == 'POST')
    assert(body.body == testBody)
   
  end

    assert(TestHttp.responseGet.headers[testHeaderName] == testHeaderValue)
    
    assert(TestHttp.responsePost.headers[testHeaderName] == testHeaderValue)
    
    
    
end
--



