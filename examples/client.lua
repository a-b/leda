-- example of aynchronous client that hits google.com every second
local client = require 'leda.client'
local http = require 'leda.client.http'

local connection = http('www.google.com')

client.timer(1, function() 
    connection:get(function(response) print(response) end)
end)
