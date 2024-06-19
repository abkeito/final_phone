#!/bin/bash

# PORT
PORT=$1

if [ -z "$PORT" ]
then
  PORT=50000
fi

for i in {1..3}
do
  echo "Sending sound/${i}.raw"
  sox -t raw -b 16 -c 1 -e s -r 44100 sound/${i}.raw -t raw -b 16 -c 1 -e s -r 44100 -| ./bin/zoom.exe 127.0.0.1 $PORT > sound/out${i}.raw &
done

wait

# play the output
for i in {1..3}
do
  echo "Playing sound/out${i}.raw"
  play -t raw -b 16 -c 1 -e s -r 44100 sound/out${i}.raw
done