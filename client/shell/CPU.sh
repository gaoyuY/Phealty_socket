#!/bin/bash
date=`date +"%Y-%m-%d__%T"`                    #时间
Average_load=`cat /proc/loadavg |cut -d " " -f 1,2,3`       #负载1 负载2 负载3
total_1=(`cat /proc/stat | sed -n "1p" | cut -d " " -f 2-9 | awk '{printf("%s %s",$1+$2+$3+$4+$5+$6+$7,$4)}'`) 
sleep 0.5s
total_2=(`cat /proc/stat | sed -n "1p" | cut -d " " -f 2-9 | awk '{printf("%s %s",$1+$2+$3+$4+$5+$6+$7,$4)}'`)
(( time_sum=${total_2[0]}-${total_1[0]}  ))    #cpu总使用时间
(( free_time=${total_2[1]}-${total_1[1]}  ))   #cpu空闲时间
cpu_percent=`echo $free_time $time_sum | awk '{printf("%.2f\n",100-$1*100/$2)}'` #cpu占用率
cpu_temperature=`cat /sys/class/thermal/thermal_zone0/temp | awk '{printf("%.2f\n",$1/1000)}'`
cpu_warning=`echo $cpu_temperature | awk 'END{if($1<50){printf("normal")}else if($1>=50&&$1<70){printf("note")}else{printf("warning")}}'`
echo $date $Average_load $cpu_percent%  $cpu_temperature°C $cpu_warning
