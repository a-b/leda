HTTP
====

To add functionality of a generic HTTP server one has to include `leda.http` module, create new  Server object and pass callback function via the `request` method. Note that callback can block and does not have to contain asynchronous code.

Server objects are created by calling the Server() function

**Server(port, host)**: creates and starts up a new HTTP server listening for incoming connections on specified host and port. If host is not provided it defaults to INADDR_ANY (listen for connections on all interfaces)

## Server objects

Fields:

**port**: listening port. 

**host**: listening host

Methods:

**request(callback)**:  function provided in the `callback` gets invoked whenever http server processes an incoming request. The function aeguments are: server object, request and response objects. 

**started(callback)**: function provided in the `callback` is invoked when server starts up.  The function argument is server object

**thread(callback)**: function provided in the `callback` is invoked whenever new connection thread is started or stopped. This function arguments are server object, thread object and a boolean flag whether thread was started or stopped

 
## Request objects

Methods:

**url**: HTTP request url

**method**: HTTP request method

**body**: contains the HTTP request body if it was sent with request

**headers**: key value table with request headers

## Response objects

Fields:

**body**: set to response body

**headers**: table with response headers

**status**: HTTP response status in numeric. defaults to 200

## Example

Consider the following example that creates HTTP server listening on localhost, port 9090 and sends a string "hello world!" in response to every request:

        local http = require('leda.http')

        local server = http.Server(9090, 'localhost')


        server:request(function(server, request, response)
            response.body = 'hello world!'
            end)
            
        server:started(function() print('server started') end)    



        





