#ifndef _COMMON_H_
#define _COMMON_H_

///
// C RunTime Header Files
//
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

//
//	STL headers
//
#include <string>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <exception> 
#include <stdexcept>  

#include "trace.h"
#include "version.h"

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif


#ifdef __MACH__
#include <CoreServices/CoreServices.h>
#endif




#endif //_COMMON_H_
