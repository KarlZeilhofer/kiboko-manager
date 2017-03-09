#!/bin/bash

###
# This script is used to debug kiboko-manager by saving the debug output, 
# the stacktrace and the network traffic to the kiboko hardware.
# 
# To use it, kiboko-manager must be compiled in debug mode.
# Put this file into the same folder as the kiboko-manager and make it executeable.
# Then run this file instead of directly starting kiboko-manager.
#
# The stacktrace and tcpdump are saved in the logs/ folder
# To inspect the recorded tcpdump you can use:
#
#	tcpdump -x -r tcpdump_<date>.bin
# or
# 	wireshark tcpdump_<date>.bin
#
###

PASSWD="feuerwehr"

# run tcpdump to log communication with timebase in background
echo $PASSWD | sudo -kS tcpdump port 10001 or port 10002 -w logs/tcpdump_$(date -Is).bin &

# start kiboko-manager with gdb and save stack trace 
gdb -batch -ex "run" -ex "shell date -In" -ex "bt full" ./kiboko-manager >logs/stacktrace_$(date -Is).txt 2>&1


# kill tcpdump
echo $PASSWD | sudo -kS pkill tcpdump
