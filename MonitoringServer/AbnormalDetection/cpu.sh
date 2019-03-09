#!/bin/bash
i=0
j=25
    while [ "$i" -lt "$j" ];
    	do
    		sleep 300s
    		time=$(date "+%Y-%m-%d %H:%M:%S")
    		echo "cpu anomaly insert ${time}" >>cpu.txt
        	siege -c 100 -t 40s http://172.17.0.2/
			let "i+=1"
    done

