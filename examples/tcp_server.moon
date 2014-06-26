-- example that starts TCP server on port 8500, that sends "hello" to every connected client every second and that echoes all  input it receives
require 'leda.tcp_server'

with TCPServer!
    .port =  12000
    .on_data = (server, connection, data) ->
        connection\send(data)
    .on_thread = (server, thread, started) ->
        if started
            server\setTimer 1, ->
                for id, connection in pairs server.connections
                    connection\send("hello")
    \start!    
