export ^

require 'leda.common'

        
-- http request class
class Request
    new: =>
        @body= __httpRequest.body
        @url= __httpRequest.url
        @method= __httpRequest.method
        @headers= __httpRequest.headers
        
        
class Response
     headers: {}
     body: ''
     status: 200
     
     new: =>
         @response = __httpResponse
         
     send: =>
         --@headers = {} if type(@headers) ~= 'table'
             
        __leda.httpResponse = {
            body: @body, 
            headers: @headers,
            status: @status
        }     
     
     
-- HTTP server class
class HTTPServer
    --default type
    type: "http"
    -- default port
    port: 8080
    -- default host
    host: '127.0.0.1'
    -- default thread count (listening for connections)
    threads: 1   
    -- default pool thread count
    pool: 1
    -- constructor
    new: =>
        if __init 
            --create server
            __api.serverCreate({
                type: @type,
                port: @port,
                host: @host,
                threads: @threads
                pool: @pool
        })
        else
            __leda.onHttpRequest = ->
                @response = Response!
                @request = Request!
                self\onRequest!    
                
                @response\send!
                
            __leda.onThreadStarted = ->
                self\onThreadStarted(Thread!)    
                
            __leda.onThreadStopped = ->
                self\onThreadStopped(Thread!)        
                
            __leda.onServerStarted = ->
                self\onStart!    
                
            __leda.onServerStopped = ->
                self\onStop!        

                
    -- method stubs                 
    onStart: =>

    onStop: =>    
        
        
    onThreadStarted: (thread) =>
        
    onThreadStopped: (thread) =>    
        
    onRequest: (request, response) =>    
        
    -- set timer. function specified in callback will be called every number of seconds specified by timeout    
    setTimer: (timeout, callback) =>
        __api.serverHttpAddTimer(timeout, false, callback)    
    
    -- set timeout. function specified in callback will be called once the timeout passes     
    setTimeout: (timeout, callback) =>
        __api.serverHttpAddTimer(timeout, true, callback)    
          
            
        
        
                
            
    
 



