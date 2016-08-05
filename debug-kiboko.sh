#!/bin/bash

PASSWD="feuerwehr"

# run tcpdump to log communication with timebase in background
echo $PASSWD | sudo -kS tcpdump port 10001 or port 10002 -w logs/dump_$(date -Is).bin &

# start kiboko-manager with gdb and save stack trace 
gdb -batch -ex "run" -ex "shell date -In" -ex "bt full" ./kiboko-manager >logs/backtrace_$(date -Is).txt 2>&1


# kill tcpdump
echo $PASSWD | sudo -kS pkill tcpdump
