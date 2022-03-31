# console-pong-cpp
Add winmm.lib to the linker in order to compile:
g++ pong.cpp -lwinmm

Alternatively:
Visual Studio -> Project properties -> Linker -> Input -> Additional Dependencies -> Add winmm.lib;

Also, may compile with
```
#pragma comment(lib, "winmm.lib")
```
inside of a source code
