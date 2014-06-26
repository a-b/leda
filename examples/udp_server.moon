-- UDP server example
require 'leda.udp_server'

with UDPServer!
    .port = 12000
    .on_data = (server, address, data) ->
        print string.format("received %s from %s", data, address)
    \start!