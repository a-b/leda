--- Asynchronous shell process
-- @usage local process = require('leda.client.process')
-- local echo = process('echo hello')
-- echo.data = function(process, data)
--     print(data)
-- end

-- @module leda.client.process

local common = require 'leda.common'

--- Background shell process class t
-- @type Process
local Process = class('Process')

local function getProcess(index)
    return __leda.processMap[index or __leda.process]
end

--- Asynchronously start  a background shell process
-- @param command command to execute
-- @name Process()
function Process:initialize(command)
    common.createClient()
    
    if __leda.init then return end
    
    self._process = __api.processStart(command)
    
    __leda.onProcessExit = __leda.onProcessExit or function()
        local process = getProcess()
        if process and type(process.exit) == 'function' then
            process:exit(__leda.processExitCode)
            __leda.processMap[process._process] = nil
        end
    end
    
    __leda.onProcessData = __leda.onProcessData or function()
        local process = getProcess()
        
        if process then
            local handler
            if __leda.processDataType == 2 then
                handler = process.error 
            elseif __leda.processDataType == 1 then
                handler = process.data
            end
            
            if type(handler) == 'function' then
                handler(process, __leda.processData)
            end
        end
    end
    
    __leda.processMap = __leda.processMap or {}
    __leda.processMap[self._process] = self
end

--- set data callback. It is called when new data is available on process stdout
-- @param callback callback function.
-- @usage process.data = function(process, data) 
-- print("process stdout", data)
--  end)
Process.data = nil

--- set error callback. It is called when new data is available on process sterr
-- @param callback callback function.
-- @usage process.data = function(process, data) 
-- print("process stderr", data)
--  end)
Process.error = nil

--- write data to process stdin
-- @param data data to write. EOF is written when data is nil 
function Process:write(data)
    if __leda.init then return end
    
    __api.processWrite(self._process, data)
end

return Process
