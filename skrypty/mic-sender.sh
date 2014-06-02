#!/bin/bash

arecord -v -t raw -f cd -B 100000 -D sysdefault | \
   ./klient -s localhost -p 1234 "$@" |
   aplay -t raw -f cd -B 5000 -v -D sysdefault - 2> /dev/null
