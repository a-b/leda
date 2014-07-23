/* 
 * functions exported to lua
 */

#ifndef API_H
#define	API_H

#include "LuaState.h"

extern int generateRandomString( lua_State* lua );
extern int clientCreate( lua_State* lua );
extern int clientConnect( lua_State* lua );
extern int clientAddTimer( lua_State* lua );
extern int clientConnectionClose( lua_State* lua );
extern int clientConnectionSendData( lua_State* lua );
extern int serverConnectionSendData( lua_State* lua );
extern int serverConnectionGetAddress( lua_State* lua );
extern int serverConnectionGetId( lua_State* lua );
extern int serverSendTo( lua_State* lua );
extern int serverCreate( lua_State* lua );
extern int serverAddTimer( lua_State* lua );
extern int getpid( lua_State* lua );
extern int processorCount( lua_State* lua );
extern int httpRequestGetUrl( lua_State* lua );
extern int httpRequestGetHeaders( lua_State* lua );
extern int httpRequestGetBody( lua_State* lua );
extern int httpRequestGetAddress( lua_State* lua );
extern int httpRequestGetMethod( lua_State* lua );
extern int httpResponseSetBody( lua_State* lua );
extern int httpResponseSetStatus( lua_State* lua );
extern int httpResponseSetHeaders( lua_State* lua );
extern int httpResponseAddHeader( lua_State* lua );
extern int getVersion( lua_State* lua );
extern int dictionarySet( lua_State* lua );
extern int dictionaryGet( lua_State* lua );
extern int dictionaryGetKeys( lua_State* lua );
extern int dictionaryRemove( lua_State* lua );
extern int processStart( lua_State* lua );
extern int processWrite( lua_State* lua );


#endif	/* API_H */

