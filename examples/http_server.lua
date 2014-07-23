-- example that starts HTTP server on port 8080 that responds with 'hello world!' to every request
local http = require('leda.server.http')

local server = http(8080, 'localhost')

server.request = function(server, request, response)
    print(string.format("request to %s from %s" , request:url(), request:address()))
    response.body =  'hello world!'
end
