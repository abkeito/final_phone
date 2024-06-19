#! /bin/bash
PORT=$1

# PORTの初期値は50000
if [ -z $PORT ]; then
  PORT=50000
fi

rec -t raw -b 16 -c 1 -e s -r 44100 - | ./bin/zoom.exe $PORT | play -t raw -b 16 -c 1 -e s -r 44100 -