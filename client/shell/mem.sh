#!/bin/bash 
if [ ! "$1" ];then
    echo ERROR
    exit
fi
date=`date +"%Y-%m-%d_%H:%M:%S"` 
total=(`free -m | head -n 2 | tail -n 1 | awk '{printf("%s %s", $2 , $3)}'`)
used=${total[1]}
((free=$total-$used))
per_percent=`echo "$total $used" | awk '{printf("%.1f", $2/$1*100)}'`
percent=`echo "$1 $per_percent" | awk '{printf("%.1f", 0.3*$1+0.7*$2)}'`
echo $date ${total[0]}M ${free}M $per_percent% $percent%
