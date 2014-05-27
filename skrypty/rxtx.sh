#!/bin/bash

sox -q "skrypty/sample.mp3" -r 44100 -b 16 -e signed-integer -c 2 -t raw - 2> /dev/null | \
   ./klient -s localhost -p 3856 "$@" | \
   aplay -t raw -f cd -B 5000 -v -D sysdefault - 2> /dev/null
