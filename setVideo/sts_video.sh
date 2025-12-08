#!/bin/bash
if [ $# -eq 2 ]; then
	echo "let's begin"
    if [ $1 -eq 7 ]; then
        ./setVideo ABCabc12 h264 $2 10.171.$1.12-26
        ./setVideo ABCabc12 h264 $2 10.171.$1.30-35
        ./setVideo ABCabc12 h264 $2 10.171.$1.40-49
    fi
    if [ $1 -eq 8 ]; then
        ./setVideo ABCabc12 h264 $2 10.171.$1.12-26
        ./setVideo ABCabc12 h264 $2 10.171.$1.30-35
        ./setVideo ABCabc12 h264 $2 10.171.$1.40-49
    fi


else
	echo "input sts NO and resolution"
fi