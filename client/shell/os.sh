#!/bin/bash
#######时间
date=`date +%Y-%m-%d_%H:%M:%S`
########主机名
hname=`hostname`
########OS版本
osname=`uname -o`
########内核版本
neiname=`uname -a | cut -d " " -f 3`
########运行时间
runtime=`uptime -p | tr -s  " " "_"`
########平均负载
adv=`cat /proc/loadavg | cut -d " " -f 1-3`
########磁盘总量
add=`df -m | grep "^/dev" | awk 'BEGIN{all=0;use=0} {all+=$2;use+=$3} END{printf("%d %d", all,use*100/all)}'`
########磁盘百分比
addbai=`echo $add | cut -d " " -f 2`
add=`echo $add | cut -d " " -f 1`
########内存大小
neicun=`free -m | grep "Mem" | tr -s " " | cut -d " " -f 2`
#########内存已用百分比
neicunbai=`free | grep "Mem" | awk '{printf("%.f", $3*100/$2)}'`
#########CPU温度
wendu=`cat /sys/class/thermal/thermal_zone0/temp | awk '{printf("%.2f", $1/1000)}'`
#########磁盘报警级别
warning=`echo $addbai $wendu $neicunbai | awk -F " " 'BEGIN{i=1} {for(; i<=3; i++){if ($i < 80){printf("normal ")}else if($i <= 90){printf("note  ")}else{printf("warning")} }}'`
#########
echo $date $hname $osname $neiname $runtime $adv $add"M" $addbai"%" $neicun"M" $neicunbai"%" $wendu"°C" $warning

