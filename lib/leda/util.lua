local addTimer = function(timeout, once, callback) 
    if __leda.init then __api.clientCreate() end

    if __leda.client then __api.clientAddTimer(timeout, once, callback) end
end

local timer = function(timeout, callback)    
    addTimer(timeout, false, callback)
end
    
    

local timeout = function(timeout, callback) 
    addTimer(timeout, true, callback)
end
   
return {timer = timer, timeout = timeout}