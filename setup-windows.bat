@echo off

if exist CMakeCache.txt del CMakeCache.txt
if exist CMakeFiles rmdir /q /s CMakeFiles

cmake -B build %*
cmake -B build %* > NUL
:: We silently run CMake twice, otherwise generated PCH files will not get grouped correctly for Visual Studio.

if %ERRORLEVEL% neq 0 (pause)