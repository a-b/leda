leda
====

high performance framework for creating powerful network applications with leightweight scripting

Features:

* All network operations are asynchronous with libevent
* Mltithreaded
* Embedded [LuaJIT](http://luajit.org) enables easy and synchronous scripting with lua or [moonscript](http://www.moonscript.org)

Installing:

    $ git clone https://github.com/sergeyzavadski/leda.git
    $ cd leda
    $ ./configure
    $ make
    $ make install
    

	Getting Started:

Example HTTP server listening on port 8080 and responding  with 'hello world!' on every request

Edit `server.lua` looks like this:

    local http = require('leda.server.http')

    local server = http(8080, 'localhost')

    server.request = function(server, request, response)
        response.body = 'hello world!'
	response:send()
    end
    
Run

    $ leda server.lua


For more usage examples see [examples](https://github.com/sergeyzavadski/leda/tree/master/examples)

More information can be found in [API Reference](http://sergeyzavadski.github.io/leda/doc) and [wiki](https://github.com/sergeyzavadski/leda/wiki)		

