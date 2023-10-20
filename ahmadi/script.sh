#!/bin/bash

log="LOG/";
rm -rf $log;
mkdir $log;
fileName="full_route_";

# ls instances
#for i in routes/*
#do
#	j=$(basename $i);

#	echo "Running allocator at $i";
#	if [ -e ${log}/${j}.log ];
#	then
#		echo "File exist...";
#	else
#		for seed in `seq 1`
#		do
			#echo "Fine"
#			./ahmadi >> ${log}/${j};
#		done
#	fi
#done

for i in `seq 1`
do
	echo "Running allocation on input number: $i";
	./ahmadi $i >> ${log}/$fileName$(($i - 1)).csv;
done