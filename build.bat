@echo off

if not exist build (
    mkdir build
)

pushd build

rem Client debug 
cl ../src/client.c /Fefps_debug.exe /Zi /Od /DDEBUG /link user32.lib gdi32.lib

rem Client release 
cl ../src/client.c /Fefps_release.exe /O2 /DNDEBUG /link user32.lib gdi32.lib

rem Run 
fps_debug.exe

popd

