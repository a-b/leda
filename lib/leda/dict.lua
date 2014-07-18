--- Key value store shared between all threads
-- @usage local dict = require('leda.dict')
-- dict.set('key', 'value')
-- local value = dict.get('key') 
-- @module leda.dict

local dict = {}

local separator = ":"

--- set key to value
-- @param key key string
-- @param value value string
dict.set = function(key, value)
    if __leda.init then return end
    
    assert(key, "no key provided")
    assert(type(value) == 'string', 'can only store strings')
    
    __api.dictionarySet(key, value)
end

--- get value 
-- @param key key string
dict.get = function(key)
    if __leda.init then return end
    
    assert(key, "no key provided")
    
    return __api.dictionaryGet(key)
end

--- delete key 
-- @param key key string
dict.delete = function(key)
    if __leda.init then return end
    
    assert(key, "no key provided")

    __api.dictionaryRemove(key)
end

return dict