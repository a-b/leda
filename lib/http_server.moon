-- example of HTTP server listening on port 9090 that responds with 'hello world' on every request
require 'leda.http_server'

class ExampleServer extends HTTPServer
    port: 9090
    
    onStart: =>
        print string.format("starting http server on port %s", @port)
        
    onRequest: =>
        print string.format("%s %s", @request.method, @request.url)
        @response.body = "hello world"
        
ExampleServer!        