#!/bin/bash

echo "$1"
if [ $1 -lt 10000000 ]
then
	echo "please input a number > 10000000 as start info!"
	exit
fi

limit=`expr $1 + 800`

ii=$1
while [ $ii -lt $limit ]
do
        echo "the $ii info process!"
        ./imclient -s 42.96.130.249 -p 5200 -b 50 -i $ii &
        #sleep 1
        ii=`expr $ii + 1`
done


echo "complete"