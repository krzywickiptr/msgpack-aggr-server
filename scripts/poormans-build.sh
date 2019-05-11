#!/bin/sh

rm -r build
mkdir build
cd build

cc -O2 --std=c11 -Wall -Wextra -pthread \
    ../src/hashtable.c \
    ../src/input.c \
    ../src/protocol.c \
    ../src/main.c \
    -o aggregation-server
