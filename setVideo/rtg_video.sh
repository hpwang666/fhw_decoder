#!/bin/bash
if [ $# -eq 2 ]; then
	echo "let's begin"
    if [ $1 -eq 13 ] || [ $1 -eq 14 ] || [ $1 -eq 15 ] || [ $1 -eq 16 ] || [ $1 -eq 17 ] || [ $1 -eq 18 ]; then
        ./setVideo ABCabc12 h264 $2 10.171.$1.24-24
        ./setVideo ABCabc12 h264 $2 10.171.$1.41-47
        ./setVideo ABCabc12 h264 $2 10.171.$1.71-76
    fi
else
	echo "input rtg NO and resolution"
fi