local dict = require 'leda.dict'
local client = require 'leda.client'
local luaunit = require 'luaunit'

local threads = 5

client.setThreads(5)

local written = 0
local read = 0

local thread = client.currentThread()

local function getData(id)
    return {
        key = string.format("key_%s", id),
        value = "value"
    }
end
--

local keys = {}
local result = {}


local write
write = function()
    while written < 10000 do
        local data = getData(written)
        dict.set(data.key, data.value)
        keys[data.key] = data.value
        written = written + 1
    end

end

client.timeout(1, write)


if thread.id == 0 then
    client.timeout(3, 
        function() 
            os.exit(luaunit:run())
        end)
end


TestDictionary = {}


TestDictionary.testKeys = function()
    local readkeys = dict.all()
    for _, key in ipairs(readkeys) do
        assert(keys[key])
        keys[key] = nil
        assert(dict.get(key))
    end
end




