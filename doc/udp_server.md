UDP
====

To add functionality of a generic UDP server one has to create new instance of `UDPServer` class with several fields initialized

## `UDPServer` class

Fields:

**port**: listening port. defaults to 8080

**host**: hostname or ip that server binds to. defaults to '127.0.0.1'. empty means accept all incoming connections

**on_data**: function that gets invoked whenever http server processes an incoming request. This function is passed three arguments: server object,address string and data received

**on_thread**: if set to a function, it gets invoked whenever new connection thread is started or stopped. This function is passed three arguments: server object , thread object and a boolean flag whether thread was started or stopped

**threads**: number of connection threads. defaults to 1

Methods:

**start()**: starts up the TCP server

**setTimer(timeout, callback)**: calls the timer to call the function provided in the  `callback` parameter every number of seconds specified by `timeout` value

**setTimeout(timeout, callback)**: calls the function provided in the  `callback` parameter once after the number of seconds provided in the `timeout` parameter passes



## Thread objects

Response objects have following fields:

**id**: thread id

## Example

Consider the following example that creates UDP server listening on port 12000 and prints a string every time it receives data

        require 'leda.udp_server'
        
        with UDPServer!
            .port = 12000
            .on_data = (server, address, data) ->
                print string.format("received %s from %s", data, address)
            \start!