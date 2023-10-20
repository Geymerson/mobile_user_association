#!/bin/bash

log="LOG/";
rm -rf $log;
mkdir $log;
fileName="full_route_";

for i in `seq 1`
do
	echo "Running allocation on input number: $i";
	./ilsvnd -i $i >> ${log}/$fileName$(($i - 1)).csv;
done