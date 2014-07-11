-- example that starts TCP server on port 10000, that sends "hello" to every connected client every second and that echoes all  input it receives
local tcp = require('leda.tcp')

local server = tcp.Server(10000, 'localhost')

server.data = function(server, connection, data)
    -- send back data receied 
    connection:send(data)
end

server.connection = function(server, connection, opened)
    if opened then print(string.format("opened connection from %s", connection:address())) end
end
    
server.thread = function(server, thread, started) 
    -- set the timer to send data to all opened connections
    server:timer(1, function() 
        for id, connection in pairs(server.connections) do
            connection:send('hello')
        end
    end)
end    
