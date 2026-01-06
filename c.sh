#!/bin/bash
mkdir -p out
g++ -O3 -march=native -ffast-math -Isrc -Iexternal src/t.cpp -o out/t
./out/t > out/o.ppm
open out/o.ppm