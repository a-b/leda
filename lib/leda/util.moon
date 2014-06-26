setTimer = (timeout, callback) ->
    __api.clientAddTimer(timeout, false, callback)
    

setTimeout = (timeout, callback) ->
    __api.clientAddTimer(timeout, true, callback)
    
{
    :setTimer, :setTimeout
}
