#!/bin/bash

if [ $1 ]; then
		threshold=$1
else
		threshold=6
fi

echo ""
echo ">>>>COMPILING<<<<"
make clean
make


echo ""
echo ">>>>EXECUTING extract_stride_data on ACCELERATION DATASET<<<<"
./extract_stride_data \
	data.csv \
	acceleration_output.csv \
	acceleration_strides.csv \
	$threshold

echo ""
echo "Lines in accel  output.csv"
wc -l acceleration_output.csv


echo ""
echo "Tail of accel output.csv"
tail acceleration_output.csv




