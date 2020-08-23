#!/bin/sh

_this=$(basename "$0")

# Get input file
if [ $# -lt 1 ] || [ $1 = '--help' ]; then
	echo "usage: $_this <inputfile> [<outputfile>]"
	exit 1
else
	_inputfile=$1
	if [ -f $_inputfile ]; then
		:
	else
		echo "$_this: Error: Could not find input file \"$_inputfile\". Exiting"
		exit 2
	fi
fi

# Get output file
if [ $# -lt 2 ]; then
	_outputfile="$_inputfile.wav"
else
	_outputfile=$2
fi

# Check if outputfile exists already
if [ -f $_outputfile ]; then
	echo "$_this: Warning: \"$_outputfile\" exists. Overwriting."
fi

# Create 'raw.wav' from 'raw.dat'
sox -b 16 -c 1 -r 44100 -e signed-integer $_inputfile $_outputfile
