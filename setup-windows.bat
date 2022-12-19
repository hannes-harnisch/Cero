@echo off

if exist CMakeCache.txt del CMakeCache.txt

cmake .
cmake . & :: CMake runs twice so that it groups generated PCH files correctly.

if %ERRORLEVEL% neq 0 (pause)