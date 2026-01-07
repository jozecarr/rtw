#!/bin/bash
mkdir -p out
BUILD=0
CONVERT=0
while [ "$1" = "--build" ] || [ "$1" = "-b" ] || [ "$1" = "--png" ]; do
  if [ "$1" = "--build" ] || [ "$1" = "-b" ]; then
    BUILD=1
  elif [ "$1" = "--png" ]; then
    CONVERT=1
  fi
  shift
done
if [ "$BUILD" -eq 1 ] || [ ! -f out/t ] || [ src/t.cpp -nt out/t ]; then
  g++ -O3 -march=native -ffast-math -Isrc -Iexternal src/t.cpp -o out/t
fi
./out/t "$@" > out/o.ppm
if [ "$CONVERT" -eq 1 ]; then
  ffmpeg -y -i out/o.ppm out/o.png > /dev/null
fi
open out/o.ppm
