Utility Functions
=================

These are the general utility function that are contained in the package  `leda.util`

**setTimer(timeout, callback)**: calls the timer to call the function provided in the  `callback` parameter every number of seconds specified by `timeout` value

**setTimeout(timeout, callback)**: calls the function provided in the  `callback` parameter once after the number of seconds provided in the `timeout` parameter passes


## Example

Consider the following example prints the string "hello world!" every second:

        util = require 'leda.util'

        util.setTimer 1, ->
            print "hello world!"
