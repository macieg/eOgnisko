#!/bin/bash

sox -q "skrypty/jama.mp3" -r 44100 -b 16 -e signed-integer -c 2 -t raw - 2> /dev/null | \
   ./klient -s localhost -p 3856 "$@"  | cat > t.out
