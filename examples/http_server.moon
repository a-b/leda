-- example of HTTP server listening on port 9090 that responds with 'hello world' on every request
require 'leda.http_server'

with HTTPServer!
    .port = 9090
    .on_request = (server, request, response)  ->
        response.body = 'hello world!'
    \start!    
