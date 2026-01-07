@echo off
setlocal
if not exist out mkdir out

set BUILD=0
if "%1"=="--build" set BUILD=1& shift
if "%1"=="-b" set BUILD=1& shift

if "%BUILD%"=="1" goto build
if not exist out\t.exe goto build
goto run

:build
g++ -O3 -march=native -ffast-math -Isrc -Iexternal src/t.cpp -o out\t.exe

:run
out\t.exe %* > out\o.ppm
start out\o.ppm
endlocal
