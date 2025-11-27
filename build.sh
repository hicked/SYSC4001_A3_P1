#!/bin/bash
set -e

BIN_DIR="bin"

if [ ! -d "$BIN_DIR" ]; then
	mkdir "$BIN_DIR"
else
	rm -rf "$BIN_DIR"/*
fi

if [[ "$1" == "--help" ]]; then
	echo "Usage: $0 [--mac]"
	echo "Compiles all .cpp files in the current directory."
	exit 0
fi

# Compile all .cpp files


# Compile each file with hardcoded output names
if [[ "$1" == "--mac" ]]; then
	g++ -g -std=c++17 -O0 -I . -o "$BIN_DIR/interrupts_EP" interrupts_101295764_101306299_EP.cpp
	echo "Built: $BIN_DIR/interrupts_EP"
	g++ -g -std=c++17 -O0 -I . -o "$BIN_DIR/interrupts_EP_RR" interrupts_101295764_101306299_EP_RR.cpp
	echo "Built: $BIN_DIR/interrupts_EP_RR"
	g++ -g -std=c++17 -O0 -I . -o "$BIN_DIR/interrupts_RR" interrupts_101295764_101306299_RR.cpp
	echo "Built: $BIN_DIR/interrupts_RR"
else
	g++ -g -O0 -I . -o "$BIN_DIR/interrupts_EP" interrupts_101295764_101306299_EP.cpp
	echo "Built: $BIN_DIR/interrupts_EP"
	g++ -g -O0 -I . -o "$BIN_DIR/interrupts_EP_RR" interrupts_101295764_101306299_EP_RR.cpp
	echo "Built: $BIN_DIR/interrupts_EP_RR"
	g++ -g -O0 -I . -o "$BIN_DIR/interrupts_RR" interrupts_101295764_101306299_RR.cpp
	echo "Built: $BIN_DIR/interrupts_RR"
fi
