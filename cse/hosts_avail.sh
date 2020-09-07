#!/bin/bash
FILE=hostfile
if [ -f $FILE ]; then
	rm $FILE
fi
prefix="172.27.19."
hosts=()
for i in 1 2 3 4 5 13 17 18 19 20 33 34 35 37 45 47 48 49 50 62 63 64 65 66 79 80 81 82 93 94 95 96 97 109 110 111 112; do
	hosts[$i]="$prefix$i"
done
number=0
for i in 1 2 3 4 5 13 17 18 19 20 33 34 35 37 45 47 48 49 50 62 63 64 65 66 79 80 81 82 93 94 95 96 97 109 110 111 112; do
	if [ $number -eq $1 ]; then
		break
	fi
	output=$(ping ${hosts[i]} -c 1 | grep "100% packet loss\|Host Unreachable" | wc -l)
	result=${output}
	if [ $result -eq 0 ]; then
		number=$((number+1))
		echo "${hosts[i]}:$2" >> hostfile
	fi
done	
