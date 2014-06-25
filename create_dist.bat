xcopy lib dist\lib /y /e
del dist\lib\moonscript

mkdir  dist\lib\moonscript

xcopy deps\moonscript\moonscript dist\lib\moonscript  /y /e
xcopy deps\luajit\src\jit\* dist\lib 
xcopy deps\luajit\src\lua51.dll* dist
xcopy deps\lpeg\lpeg.dll dist\lib
xcopy deps\lfs\lfs.dll dist\lib
xcopy obj\leda.exe dist	