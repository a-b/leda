leda
====

high performance framework for creating powerful network applications with leightweight scripting

Features:

* Easy to build high pefomance HTTP, UDP or TCP servers and asynchronous TCP and HTTP clients
* Synchronous scripting with lua 
* Super fast VM with LuaJIT(http://luajit.org)
* Multithreaded process architecture and asynchronous networking with libevent (via https://github.com/sergeyzavadski/libpropeller)
* Cross platform (Unix and Windows)

Installing on Unix:

    $ git clone https://github.com/sergeyzavadski/leda.git
    $ cd leda
    $ ./configure
    $ make
    $ make install
    

Running:

Example HTTP server listening on port 8080 and responding  with 'hello world!' on every request

Edit `server.lua` looks like this:

    local http = require('leda.http')

    local server = http.Server(8080, 'localhost')

    server.request = function(server, request, response)
        response.body = 'hello world!'
    end
    
Run

    $ leda server.lua


For more usage examples see [examples](https://github.com/sergeyzavadski/leda/tree/master/examples)

More information can be found in [API Reference](http://sergeyzavadski.github.io/leda/) and [wiki](https://github.com/sergeyzavadski/leda/wiki)		

