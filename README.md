leda
====

high performance framework for creating powerful network applications with leightweight scripting

Features:

* Synchronous scripting with moonscript (http://www.moonscript.org)
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

Example HTTP server listening on port 9090 and responding  with 'hello world!' on every request

Edit `server.moon` looks like this:

    -- example of HTTP server listening on port 9090 that responds with 'hello world' on every request
    require 'leda.tcp_server'
    
    with HTTPServer!
        .port = 9090
        .on_request = (server, request, response)  ->
            response.body = 'hello world!'
        \start!    
    
Run

    $ leda server.moon


For more examples see [examples](https://github.com/sergeyzavadski/leda/tree/master/examples)

For more information see [API Reference](https://github.com/sergeyzavadski/leda/tree/master/doc) or [wiki](https://github.com/sergeyzavadski/leda/wiki)		

