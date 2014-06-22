require 'leda.tcp_server'

--
class MyServer extends TCPServer
    port: 12000

    onDataReceived: (connection, data) =>
        connection\send(data)

    onThreadStarted: (thread) =>
        

        --self\setTimeout(1, callback)

        callback = ->
            for id, connection in pairs @connections
                connection\send("timer")

--        self\setTimer(1, callback)

--
--
--
MyServer!
-- --    \test!
--
--
--
--
--
--
--
--
