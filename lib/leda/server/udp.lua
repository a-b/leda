--- UDP server functionality
-- @usage local udp = require('leda.server.udp')
-- local server = udp(10000, 'localhost')
-- server.data = function(server, from, data)
--     print(string.format("received %s from %s", data, from))
-- end
--
-- @module leda.server.udp

local common = require ('leda.common')


--- server class
-- @type Server
local Server = class('Server', common.Server)

--- Create a server instance
-- @param port listening port
-- @param[opt] host listening host (defaults to 'localhost')
-- @return a new Server object
-- @usage local server = Server(12000, 'some.domain')
-- @name Server()
function Server:initialize(port, host)
    self.type = 'udp'
    self.port = port or 12000
    self.host = host or 'localhost'

    
    __leda.onUdpDataReceived = function()
        if type(self.data) == 'function' then
            self:data(__leda.from, __leda.data)
        end
    end
    
    common.Server.initialize(self)
end

--- send to address
-- @param host address host
-- @param port address port
-- @param data data to send
function Server:send(host, port, data)
    if data then __api.serverSendTo(host, port, data) end
end

--- data callback. runs when new data has beem received by server
-- @param callback function
-- @usage server.data = function(server, from, data)
--    print(string.format("received %s from %s", data, from))
-- end
Server.data = nil


--- thread callback. runs when thread has been started or stopped
-- @param callback callback function
-- @usage server.thread = function(server, thread, started) 
--      local verb = "started"
--      if stopped then verb = "stopped" end    
--      print(string.format("thread %s was %s", thread.id, verb))
--  end)
Server.thread = nil

--- set timeout. calls the function once after the seconds value elapses
-- @param seconds number of seconds 
-- @param callback function
function Server:timeout(...)
    common.Server.timeout(self, ...)
end    

--- set timer. calls the function every time the seconds value elapses
-- @param seconds number of seconds 
-- @param callback function
function Server:timer(...)
    common.Server.timer(self, ...)
end    

local function send(host, port, data)
    -- todo: implement
end
        
return Server