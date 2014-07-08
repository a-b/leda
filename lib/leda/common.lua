-- thread class
Thread = class('Thread')

function Thread:initialize()
    self.id = __leda.threadId
end

-- common server class
local Server = class('Server')

function Server:initialize()
    assert(self.type, "self.type not set")
    assert(self.host, "self.host not set")
    assert(self.port, "self.port not set")
    
    if __leda.init then
        -- create the server via native api
        __api.serverCreate({
            type = self.type,
            port = self.port,
            host = self.host,
            threads = __api.processorCount()
            })
    else
        -- set callbacks
        __leda.onThreadStarted = function()
            if __leda.threadId == 0 then
                if type(self.started) == 'function' then self:started()  end
            end
                            
            if type(self.thread) == 'function' then self:thread(Thread(), true) end
        end
        
        __leda.onThreadStopped = function()
            if type(self.thread) == 'function' then self:thread(Thread(), false) end
        end
    end
end

-- set timer
function Server:timer(timeout, callback)
    if type(callback) == 'function' then
        __api.serverAddTimer(timeout, false, callback)
    end
end

-- set timeout
function Server:timeout(timeout, callback)
    if type(callback) == 'function' then
        __api.serverAddTimer(timeout, true, callback)
    end
end


return {Thread=Thread, Server=Server, Utility=Utility}