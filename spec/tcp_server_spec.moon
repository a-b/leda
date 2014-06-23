describe "tcp server", ->
  setup -> 
      socket = require 'socket'
      lfs = require 'lfs'
      print string.format("%s/%s", lfs.currentdir(), "lib")
      os.execute(string.format('LEDA_PATH="%s/%s" obj/leda examples/tcp_server.moon &', lfs.currentdir(), "lib"))
      os.execute("sleep 1")
      
  it "should listen on port 8500", ->
      tcp = socket.tcp()
      assert(tcp\connect("127.0.0.1", 8500))
      tcp\close()
  
  it "should echo data it receives", ->
     tcp = socket.tcp()
     tcp\connect("127.0.0.1", 8500)
     string = 'test test'
     tcp\send(string.format("%s\n", string))
     res =  tcp\receive('*l')
     assert.are.equals(res, string)

     tcp\close()
         
  it "should send a string to all clients", ->
      tcp = socket.tcp()
      tcp\connect("127.0.0.1", 8500)
      
      res =  tcp\receive('*l')
      assert.are.equals(res, 'hello')
      tcp\close()
      
  teardown ->
    os.execute("killall leda")  
