setTimer = (timeout, callback) ->
    set = __threadId == 0 or __init
        
    __api.clientAddTimer(timeout, false, callback) if set
    

setTimeout = (timeout, callback) ->
    set = __threadId == 0 or __init
        
    __api.clientAddTimer(timeout, true, callback) if set
    
{
    :setTimer, :setTimeout
}
