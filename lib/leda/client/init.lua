---  Asynchronous client functionality
-- @module leda.client
local common = require 'leda.common'

local client = {}

-- util functions

local addTimer = function(timeout, once, callback) 
    common.createClient()
    
    if not __leda.init and __leda.client then __api.clientAddTimer(timeout, once, callback) end
end

--- set client threads
-- @param count client thread count (defaults to 1)
function client.setThreads(count)
    common.createClient(count)
end

--- get current thread
-- @return current thread
function client.currentThread()
    return common.Thread()
end

--- set timer. calls the function every time the seconds value elapses
-- @param timeout. can be a number of seconds or a table with one or more keys: 'sec' specifying seconds, 'msec' specifying milliseconds and 'usec' specifying  microseconds
-- @param callback function
client.timer = function(timeout, callback)    
    addTimer(timeout, false, callback)
end

--- set timeout. calls the function once after the seconds value elapses
-- @param timeout. can be a number of seconds or a table with one or more keys: 'sec' specifying seconds, 'msec' specifying milliseconds and 'usec' specifying  microseconds
-- @param callback function
client.timeout = function(timeout, callback) 
    addTimer(timeout, true, callback)
end

return client