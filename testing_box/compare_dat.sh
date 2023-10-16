#! /bin/bash

for i in `seq 67 5368`;
do
		diff "./sender/"${i}".dat" "./receiver2/"${i}".dat" > /dev/null 2>&1
		error=$?
		if [ $error -eq 1 ]
		then
   			echo "sender/"${i}".dat and receiver2/"${i}".dat differ"
   		else
   			diff "./sender/"${i}".dat" "./receiver2/"${i}".dat"
   		fi
done
