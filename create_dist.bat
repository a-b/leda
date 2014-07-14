xcopy lib dist\lib /y /e
xcopy deps\luajit\src\jit\* dist\lib 
xcopy deps\luajit\src\lua51.dll dist
xcopy deps\luajit\src\lua51.lib dist

xcopy deps\luajit\src\luajit.exe dist				

mkdir dist\include
xcopy deps\luajit\src\lua.h dist\include 	 

xcopy leda.exe dist	