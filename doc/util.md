util
====

These are the general utility function that are contained in the package  `leda.util`

**timer(timeout, callback)**: calls the timer to call the function provided in the  `callback` parameter every number of seconds specified by `timeout` value

**timeout(timeout, callback)**: calls the function provided in the  `callback` parameter once after the number of seconds provided in the `timeout` parameter passes


## Example

Consider the following example prints the string "hello world!" every second:

        local util = require('leda.util') 

        util.timer(1, function() print('hello world!') end)
