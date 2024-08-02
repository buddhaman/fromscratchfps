IF NOT EXIST build (
    mkdir build
)

pushd build

rem Client debug 
cl ../src/client.c /Fefps_debug.exe /Zi /Od /DDEBUG /link user32.lib gdi32.lib Ws2_32.lib
IF %ERRORLEVEL% NEQ 0 (
    echo Debug build failed.
    popd
    EXIT /B 1
)

rem Client release 
cl ../src/client.c /Fefps_release.exe /O2 /DNDEBUG /link user32.lib gdi32.lib
IF %ERRORLEVEL% NEQ 0 (
    echo Release build failed.
    popd
    EXIT /B 1
)

rem Server debug 
cl ../src/server.c /Feserver_debug.exe /Zi /Od /DDEBUG /link user32.lib gdi32.lib Ws2_32.lib
IF %ERRORLEVEL% NEQ 0 (
    echo Debug build failed.
    popd
    EXIT /B 1
)

REM Run 
fps_debug.exe

popd
