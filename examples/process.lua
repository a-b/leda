-- example that launches background shell asynchronously
local process = require 'leda.client.process'

process = process('echo hello')

process.data = function(process, data)
    print("data from process", data)
end

process.exit = function(process, code)
    print("process exited with code", code)
end
