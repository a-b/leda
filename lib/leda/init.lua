-- lua std lib
require 'std'
-- middleclass
require 'middleclass'

-- reference storage global table
__leda = __leda or {}

for key, value in pairs(__api) do
    __leda[key] = value
end 
