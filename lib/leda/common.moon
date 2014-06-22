export ^

class Thread
    new: => 
        @thread = __thread
        @id = __api.threadGetId(@thread)
