#!/bin/bash
if [ $# -ne 4 ] 
then
	echo wrong arguments
fi

if [ ! -e $1 ]
then 
	echo Directory $1 not exists
fi

if [ ! -e $2 ]
then
	echo text file $2 not exists
fi

re='^[0-9]+$'
if ! [[ $3 =~ $re ]] || ! [[ $4 =~ $re ]] ; then
   echo "error: Not a number" >&2; exit 1
fi

a=$(wc -l $2 | cut -d " " -f 1)
if ((a < 10000))
then
	echo less than 10000
fi

#Create w directories 
for (( i=0 ; i< $3 ; i++)) do 
	printf -v var 'site%d' "$i"
	mkdir "$var"
done
i=0
for dir in "$(find . -type d)"
do
	echo "$dir"
	#//cd "$dir"
	for (( j=0 ; j< $4 ; j++)) do
		printf -v var 'page%d_%d' "$i" "$RANDOM"
		echo "$dir/$var"
		touch "${dir}/$var"
	done
	cd ..
	let "i = i  + 1"
done
