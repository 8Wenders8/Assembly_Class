#!/bin/bash
gcc -Wall -g -o wsh wsh.c common.c conn.c args.c 
rm /tmp/sckt
