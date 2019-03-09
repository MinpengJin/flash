#!/bin/bash
i=0
j=25
    while [ "$i" -lt "$j" ];
    	do
    		fio -filename=/home/t1.img -direct=1 -iodepth 1 -thread -rw=randwrite -ioengine=psync -bs=4k -size=200M -numjobs=10 -runtime=300 -group_reporting -name=mytest
    		time=$(date "+%Y-%m-%d %H:%M:%S")
    		echo "disk anomaly insert ${time}" >>disk.txt
    		fio -filename=/home/t1.img -direct=1 -iodepth 1 -thread -rw=randwrite -ioengine=psync -bs=48k -size=200M -numjobs=10 -runtime=40 -group_reporting -name=mytest
			let "i+=1"
    done

