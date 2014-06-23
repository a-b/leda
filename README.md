leda
====

high performance framework for creating powerful network applications with leightweight scripting

Features:

* Synchronous scripting with moonscript (http://www.moonscript.org)
* Super fast VM with LuaJIT(http://luajit.org)
* Multithreaded process architecture and asynchronous networking with libevent (via https://github.com/sergeyzavadski/libpropeller)

Installing:

    $ git clone https://github.com/sergeyzavadski/leda.git
    $ cd leda
    $ ./configure
    $ make
    $ sudo make install
    

Running:

Example HTTP server listening on port 9090 and responding  with 'hello' on every request

Edit `server.moon` looks like this:

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

Run

    $ leda server.moon


For more examples see [examples](https://github.com/sergeyzavadski/leda/tree/master/examples)
		

