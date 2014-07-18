local export = {}

local separator = ":"
local addValue

export.set = function(key, value)
    if __leda.init then return end
    
    assert(key, "no key provided")
    assert(type(value) == 'string', 'can only store strings')
    
    __api.dictionarySet(key, value)
end


export.get = function(key)
    if __leda.init then return end
    
    assert(key, "no key provided")
    
    return __api.dictionaryGet(key)
end

export.delete = function(key)
    if __leda.init then return end
    
    assert(key, "no key provided")

    __api.dictionaryRemove(key)
end

return export