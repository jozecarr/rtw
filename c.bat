@echo off
if not exist out mkdir out
g++ -O3 -march=native -ffast-math -Isrc -Iexternal src/t.cpp -o out/t.exe
out\t.exe > out\o.ppm
start out\o.ppm