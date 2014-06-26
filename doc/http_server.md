HTTP
====

To add functionality of a generic htttp server one has to create new instance of `HttpServer` class with `on_request` field initialized

## `HttpServer` class

Fields:

**port**: listening port. defaults to 8080

**host**: hostname or ip that server binds to. defaults to '127.0.0.1'. empty means accept all incoming connections

**on_request**: function that gets invoked whenever http server processes an incoming request. This function is passed three arguments: server object, request and response.

**threads**: number of connection threads. defaults to 1

**pool**: number of pool (request processing) threads. defaults to 1

Methods:
**start()**: starts up the http server

 
## Request objects

Request objects have following fields:

**url**: HTTP request url

**method**: HTTP request method

**body**: contains the HTTP request body if it was sent with request

**headers**: key value table with request headers

## Response objects

Response objects have following fields:

**body**: set to response body

**headers**: table with response headers

**status**: HTTP response status in numeric. defaults to 200

## Example

Consider the following example that creates HTTP server listening on port 9090 and sends a string "hello world!" in response to every request:

        with HTTPServer!
            .port = 9090
            .on_request = (server, request, response)  ->
                response.body = 'hello world!'
            \start!    





