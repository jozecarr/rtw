#!/bin/bash
mkdir -p out
BUILD=0
if [ "$1" = "--build" ] || [ "$1" = "-b" ]; then
  BUILD=1
  shift
fi
if [ "$BUILD" -eq 1 ] || [ ! -f out/t ] || [ src/t.cpp -nt out/t ]; then
  g++ -O3 -march=native -ffast-math -Isrc -Iexternal src/t.cpp -o out/t
fi
./out/t "$@" > out/o.ppm
open out/o.ppm
