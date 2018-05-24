#!/bin/bash

if [ $# -ne 1 ] 
then
	echo give port;
fi
rm -rf ./scr/*
cd ./objs
gcc -c -g ~/jobExecutor/tests/sdi1500076/*.c
cd ..
gcc -g crawler.c *.o objs/* -pthread -lm -o crawler
gdb --args ./crawler 127.0.0.1 7777 /site0/page0_7260.html $1
