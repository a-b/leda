local luaunit = require 'luaunit'
local tcp = require 'leda.tcp'
local client = require 'leda.client'

TestTcp = {stringsSent = {}, stringsReceived = {}}

local testString = 'test'
local helloString = 'hello'
local host = 'localhost'

math.randomseed(os.time())
local port = 10000 + math.random(1000)

local connection = client.Connection(host, port)

TestTcp.sendStrings = function()
    if #TestTcp.stringsSent < 2000 then
        local s = string.random(100)

            connection:send(s)
            
            table.insert(TestTcp.stringsSent, s)
            
            client.timeout(0, function()
                TestTcp.sendStrings()
            end)
        end
end

connection.opened = function(connection)
    TestTcp.openedConnection = true
    connection:send(testString)
    
    client.timeout(0, function()
        TestTcp.sendStrings() end)
end

connection.error = function(connection)
    client.timer(1, function()
        connection:_connect(host, port)
    end)

end

connection.data = function(connection, data)
    TestTcp.helloString = TestTcp.helloString or data:find(helloString) 
    TestTcp.testString = TestTcp.testString or data:find(testString) 
    
    for i, s in ipairs(TestTcp.stringsSent) do
        if data:find(s) then 
            table.insert(TestTcp.stringsReceived, s)
         end
    end
end

local server = tcp.Server(port, host)

server.data = function(server, connection, data)
    connection:send(data)
end
        
server.thread = function(server, thread)
    server:timer(1, function()
        for _, connection in pairs(server.connections) do
            connection:send(helloString)
        end
    end)
end

client.timeout(3, function()
        os.exit(luaunit:run())
        end)
    
function TestTcp:testConnect()
    assert(TestTcp.openedConnection)
    assert(TestTcp.helloString)
    assert(TestTcp.testString)
    
    assert(#TestTcp.stringsReceived >= #TestTcp.stringsSent)
end
--



