-- utility functions
local export = {}

-- format time 
export.formatTime = function(timestamp, format, tzoffset, tzname)
    format = format or "!%c %z"
    tzoffset = tzoffset or  "GMT"
    
    if tzoffset == "local" then  -- calculate local time zone (for the server)
       local now = os.time()
       local local_t = os.date("*t", now)
       local utc_t = os.date("!*t", now)
       local delta = (local_t.hour - utc_t.hour)*60 + (local_t.min - utc_t.min)
       local h, m = math.modf( delta / 60)
       tzoffset = string.format("%+.4d", 100 * h + 60 * m)
    end
   
    tzoffset = tzoffset or "GMT"
    format = format:gsub("%%z", tzname or tzoffset)
    if tzoffset == "GMT" then
       tzoffset = "+0000"
    end
   
    tzoffset = tzoffset:gsub(":", "")

    local sign = 1
    if tzoffset:sub(1,1) == "-" then
       sign = -1
       tzoffset = tzoffset:sub(2)
    elseif tzoffset:sub(1,1) == "+" then
       tzoffset = tzoffset:sub(2)
    end
   
    tzoffset = sign * (tonumber(tzoffset:sub(1,2))*60 +  tonumber(tzoffset:sub(3,4))) * 60
   
    return os.date(format, timestamp + tzoffset)
end


return export