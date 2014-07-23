-- UDP server example 
local udp = require('leda.server.udp')

local server = udp(10000, 'localhost')

server.started = function() print('server started') end

server.data = function(server, from, data)
    print(string.format("received %s from %s", data, from))
end
