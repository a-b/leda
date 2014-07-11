-- example that starts HTTP server on port 8080 that responds with 'hello world!' to every request
local http = require('leda.http')

local server = http.Server(8080, 'localhost')

server.request = function(server, request, response)
    print(string.format("request to %s from %s" , request:url(), request:address()))
    print(string.format("headers: %s, body: %s", tostring(request:headers()), request:body()))
    response.body =  'hello world!'
end
