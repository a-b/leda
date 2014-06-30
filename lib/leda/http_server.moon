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
         @headers = {}
         
     send: =>
         --@headers = {} if type(@headers) ~= 'table'
             
        __leda.httpResponse = {
            body: @body, 
            headers: @headers,
            status: @status
        }     
     
     
-- HTTP server class
class HTTPServer extends CommonServer
    --default type
    type: "http"
    -- default port
    port: 8080
    -- default  thread count
    pool: math.floor(__api.processorCount() / 1.5)
    threads: math.floor(__api.processorCount() / 3)
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
                pool: @pool
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
                
        
                                
            
    
 




