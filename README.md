leda
====

high performance framework for creating powerful network applications with leightweight scripting

Features:

* Extremely fast
* Synchronous scripting with lua 
* Super fast VM with LuaJIT(http://luajit.org)
* Multithreaded process architecture and asynchronous networking with libevent (via https://github.com/sergeyzavadski/libpropeller)
* Cross platform (Unix and Windows)

Installing on Unix:

    $ git clone https://github.com/sergeyzavadski/leda.git
    $ cd leda
    $ ./configure
    $ make
    $ sudo make install
    

Running:

Example HTTP server listening on port 8080 and responding  with 'hello world!' on every request

Edit `server.lua` looks like this:

    local http = require('leda.http')

    local server = http.Server(8080, 'localhost')


    server:request(function(request, response)
        response.body = 'hello world!'
    end)
    
Run

    $ leda server.lua


For more usage examples see [examples](https://github.com/sergeyzavadski/leda/tree/master/examples)

More information can be found in [API Reference](https://github.com/sergeyzavadski/leda/tree/master/doc) and [wiki](https://github.com/sergeyzavadski/leda/wiki)		

