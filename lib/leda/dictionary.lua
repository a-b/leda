local export = {}

local addValue
addValue = function(key, value)
    if type(value) == 'table' then
        for name, field in pairs(value) do
            addValue(key .. "_" .. name, field)
        end
    else
        __leda.dictionarySet(key, value)        
    end
    
end


export.set = function(key, value)
    if __leda.init then return end
    
    if not key or not value then return end
    
    addValue(key, value)
end


export.getkeys = function(key)
    if __leda.init then return end
    
    if not key then return end
    
    return __leda.dictionaryGetKeys(key)
end


local TableParser = class('TableParser')
function TableParser:initialize(key, keys)
    self.prefix = key .. "_"
    self.key = key
    self.keys = keys
    self.table = {}
    
    self:_next()
end

function TableParser:_next()
    if not next(self.keys) then 
        return 
    end
    
    local index, key = next(self.keys)
    local fullkey = key
    local value = export.get(fullkey)
    
    local s,e = key:find(self.prefix)
    key = key:sub(e + 1)
    
    local parts = key:split("_")
    
    local table = self.table
    local parent
    
    for _, part in ipairs(parts) do
        if parent then 
            table[parent] = {}
            table = table[parent]
        end
        parent = part
    end
    
    
    table[parent] = value
    self.keys[index] = nil
    self:_next()
end

export.get = function(key)
    if __leda.init then return end
    
    if not key then return end
    
    local value = __leda.dictionaryGet(key)
    if not value then
        local keys = export.getkeys(key)
        if not next(keys) then return end
        
        local result = {}
        
        local parser = TableParser(key, keys)
        return parser.table
    else
        return value
    end
end


return export