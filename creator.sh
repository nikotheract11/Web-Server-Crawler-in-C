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

LINES=$(wc -l $2 | cut -d " " -f 1)
if ((LINES < 10000))
then
	echo less than 10000
fi

#Create w directories
for (( i=0 ; i< $3 ; i++)) do
	printf -v var 'site%d' "$i"
	mkdir "$var"
done

# Create p pages for each dir
i=0
for dir in */
do
	echo "$dir"
	for (( j=0 ; j< $4 ; j++)) do
		printf -v var 'page%d_%d.html' "$i" "$RANDOM"
		touch "$dir""$var"
	done
	let "i = i  + 1"
done

k=0
let "k=$RANDOM%(LINES-2000)"
echo "k=$k"

m=0
let "m=$RANDOM%(1000)+1000"
echo "m=$m"

# https://stackoverflow.com/questions/16487258/how-to-declare-2d-array-in-bash


let "f= ($4/2) + 1"
let "q= ($3/2) + 1"
echo "f=$f, q=$q"

for dir in */
do
   cd "$dir"
   for file in *
   do
      if [ -e ../links ]
      then
         rm -rf ../links
      fi
      #echo ok #\<html\> >> $file ; echo \<body\> >> $file
      for ((i=0;i<$f;i++)) do
         link=$(ls | shuf | head -n 1)
         while [ "$link" == "$file" ]
         do
            link=$(ls | shuf | head -n 1)
         done
         echo "../$dir$link" >> ../links
      done
      #for ((i=0;i<$q;i++)) do
      for di in ../*/
      do
         if [ "$di" == "../$dir" ]
         then
            continue
         fi
         for fil in "$di"*
         do
            #sub=$(echo "$fil" | cut -d "." -f 3)
            #echo "$sub.html" >> ../ext
            echo "$fil" >> ../ext
         done
      done
      cat ../ext
      shuf ../ext | head -n "$q" >> ../links
      echo
      rm -rf ../ext
      cat ../links >> ../total

      for ((i=1;i<=($f+$q);i++))
      do
         awk 'NR>=(k+i*(m/(f+q)))&&NR<=(k+(i+1)*m/(f+q))' "../$2" >> $file
         echo >> $file
         a=$(sed -n -e "$i"p "../links")
         echo "<a href=$a> Link$i </a>" >> $file
      done
      echo \</body\> >> $file ; echo \</html\> >> $file

   done
   cd ..
done
