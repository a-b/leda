xcopy lib dist\lib /y /e
xcopy deps\luajit\src\jit\* dist\lib 
xcopy deps\luajit\src\lua51.dll* dist
xcopy leda.exe dist	