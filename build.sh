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
	g++ -g -std=c++17 -O0 -I . -o "$BIN_DIR/interrupts_EP" interrupts_student1_student2_EP.cpp
	g++ -g -std=c++17 -O0 -I . -o "$BIN_DIR/interrupts_EP_RR" interrupts_student1_student2_EP_RR.cpp
	g++ -g -std=c++17 -O0 -I . -o "$BIN_DIR/interrupts_RR" interrupts_student1_student2_RR.cpp
else
	g++ -g -O0 -I . -o "$BIN_DIR/interrupts_EP" interrupts_student1_student2_EP.cpp
	g++ -g -O0 -I . -o "$BIN_DIR/interrupts_EP_RR" interrupts_student1_student2_EP_RR.cpp
	g++ -g -O0 -I . -o "$BIN_DIR/interrupts_RR" interrupts_student1_student2_RR.cpp
fi
echo "Built: $BIN_DIR/interrupts_EP"
echo "Built: $BIN_DIR/interrupts_EP_RR"
echo "Built: $BIN_DIR/interrupts_RR"