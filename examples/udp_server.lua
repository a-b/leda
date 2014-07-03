-- UDP server example 
local udp = require('udp')

local server = udp.Server(10000, 'localhost')

server:started(function() print('server started') end)

server:data(function(from, data)
    print(string.format("received %s from %s", data, from))
    end)