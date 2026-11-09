@echo off
REM MinGW-w64 (e.g. MSYS2 or Code::Blocks bundled MinGW) + FreeGLUT in your MinGW folders
g++ -std=c++11 -O2 -Iinclude src\*.cpp -o CleanCitySimulator.exe -lfreeglut -lopengl32 -lglu32
if errorlevel 1 (echo BUILD FAILED) else (echo Built CleanCitySimulator.exe & CleanCitySimulator.exe)
