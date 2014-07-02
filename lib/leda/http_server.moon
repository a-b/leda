export ^

require 'leda.common'

        
-- http request class
class Request    
    url: => __api.httpRequestGetUrl(__leda.httpRequest)
    headers: => __api.httpRequestGetHeaders(__leda.httpRequest)
    body: => __api.httpRequestGetBody(__leda.httpRequest)
    
        
        
        
class Response
     headers: {}
     body: ''
     status: 200
     
     new: =>
         @headers = {}
         @body = ''
         
     send: =>
        __api.httpResponseSetStatus(__leda.httpResponse, @status)
        __api.httpResponseSetHeaders(__leda.httpResponse, @headers)
        __api.httpResponseSetBody(__leda.httpResponse, @body)
        
                     
-- HTTP server class
class HTTPServer extends CommonServer
    --default type
    type: "http"
    -- default port
    port: 8080
    -- default  thread count
    threads: __api.processorCount() 
    -- constructor
    new: =>
        __leda.onHttpRequest = ->
            @response = Response!
            @request = Request!
            self\onRequest!

            @response\send!

        __leda.onServerStarted = ->
            self\onStart!

        __leda.onServerStopped = ->
            self\onStop!


                
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
        
        
    onStart: =>    
            
    onStop: =>    
        
    onRequest:  =>
        if @on_request
            @on_request(@request, @response)
            
        -- set some headers and serialize tables to json
        if not @response.headers['Content-Type']
            local contentType
            if type(@response.body) == 'table'
                contentType = 'application/json'
                @response.body = json.encode(@response.body)
            else
                contentType = 'text/plain'

            @response.headers['Content-Type'] = contentType

        @response.headers['Date'] = __leda.formatTime(os.time())
                
        
                                
            
    
 




