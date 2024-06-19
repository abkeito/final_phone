#! /bin/bash
IP=$1
PORT=$2

if [ -z $IP ]; then
  IP="127.0.0.1"
fi

if [ -z $PORT ]; then
  PORT=50000
fi

rec -t raw -b 16 -c 1 -e s -r 44100 - | ./bin/zoom.exe $IP $PORT | play -t raw -b 16 -c 1 -e s -r 44100 -