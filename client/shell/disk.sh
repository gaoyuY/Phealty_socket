#!/bin/bash
date=`date +"%Y-%m-%d_%T"` 
arr=(`df -m | grep "^/dev" | awk '{printf(" %s %s %s %s %s\n",$2,$3,$4,$6,$5)}'`)
for (( i=0;i<${#arr[@]};i+=5 ));do
	(( stay=${arr[i]}-${arr[i+1]} ))
     echo $date "1" ${arr[i+3]} ${arr[i]} $stay ${arr[i+4]}  
     (( disk_sum=$disk_sum+${arr[i]} ))
     (( disk_used=$disk_used+${arr[i+1]} ))
done

(( disk_stay=$disk_sum-$disk_used ))
(( disk_percent=$disk_used*100/$disk_sum ))

echo $date "0" "disk" $disk_sum $disk_stay $disk_percent%
