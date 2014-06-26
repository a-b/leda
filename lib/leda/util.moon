addTimer = (timeout, once, callback) ->
    __api.clientCreate() if __init    
    __api.clientAddTimer(timeout, once, callback) if __client

setTimer = (timeout, callback) ->    
    addTimer(timeout, false, callback)
    
    

setTimeout = (timeout, callback) ->
    addTimer(timeout, true, callback)
    
{
    :setTimer, :setTimeout
}
