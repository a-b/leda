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

-- taken from LuaSocket
export.parseUrl = function(url)
    -- initialize default parameters
    local parsed = {}
    -- empty url is parsed to nil
    if not url or url == "" then return nil, "invalid url" end
    -- remove whitespace
    -- url = string.gsub(url, "%s", "")
    -- get fragment
    url = string.gsub(url, "#(.*)$", function(f)
        parsed.fragment = f
        return ""
    end)
    -- get scheme
    url = string.gsub(url, "^([%w][%w%+%-%.]*)%:",
        function(s) parsed.scheme = s; return "" end)
    -- get authority
    url = string.gsub(url, "^//([^/]*)", function(n)
        parsed.authority = n
        return ""
    end)
    -- get query string
    url = string.gsub(url, "%?(.*)", function(q)
        parsed.query = q
        return ""
    end)
    -- get params
    url = string.gsub(url, "%;(.*)", function(p)
        parsed.params = p
        return ""
    end)
    -- path is whatever was left
    if url ~= "" then parsed.path = url end
    local authority = parsed.authority
    if not authority then return parsed end
    authority = string.gsub(authority,"^([^@]*)@",
        function(u) parsed.userinfo = u; return "" end)
    authority = string.gsub(authority, ":([^:%]]*)$",
        function(p) parsed.port = p; return "" end)
    if authority ~= "" then 
        -- IPv6?
        parsed.host = string.match(authority, "^%[(.+)%]$") or authority 
    end
    local userinfo = parsed.userinfo
    if not userinfo then return parsed end
    userinfo = string.gsub(userinfo, ":([^:]*)$",
        function(p) parsed.password = p; return "" end)
    parsed.user = userinfo
    return parsed
end

return export