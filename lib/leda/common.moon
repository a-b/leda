export ^

-- Thread class
class Thread
    new: => 
        @id = __threadId
        
-- Common server class
class CommonServer
    -- default port
    port: 8000
    -- default host
    host: 'localhost'
    -- default thread count
    threads: __api.processorCount()
    
    -- constructor
    new: =>
        -- set callback functions
        __leda.onThreadStarted = ->
            self\onThreadStarted(Thread!)
    
        __leda.onThreadStopped = ->
            self\onThreadStopped(Thread!)    

 
    -- starts the server
    start: =>
        if __init 
            --create server
            __api.serverCreate({
                type: @type,
                port: @port,
                host: @host,
                threads: @threads
        })
                 

    -- new thread was started
    onThreadStarted: (thread) =>
        if @on_thread
            @on_thread(thred, true)

    -- thread was stopped 
    onThreadStopped: (thread) =>
        if @on_thread
            @on_thread(thread, false)



    -- set timer. function specified in callback will be called every number of seconds specified by timeout    
    setTimer: (timeout, callback) =>
        __api.serverAddTimer(timeout, false, callback)    

    -- set timeout. function specified in callback will be called once the timeout passes     
    setTimeout: (timeout, callback) =>
        __api.serverAddTimer(timeout, true, callback)    

    

