#!/bin/bash
per_date=`date +"%Y-%m-%d_%T"`  #时间

yonghu_num=`cat /etc/passwd |grep "bash"| wc -l`    #用户总数（非系统用户）

sudo_name=`cat /etc/group | sort | grep "sudo" |cut -d ":" -f 4`        #具有Root权限用户                                      

now_yonghu=` w -h | awk '{print $1"_"$3"_"$2}' | sort | uniq -c | sort -n |awk '{print $2}' |xargs | tr " " ","`  #当前在线用户

active_num=`last |cut -d " " -f 1 | head -n -2 | sort |uniq -c |tail -n 3 | awk '{print $2}'|xargs |tr " " ","`  #近期活跃用户

echo $per_date $yonghu_num [$active_num] [$sudo_name] [$now_yonghu]
