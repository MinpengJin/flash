#!/bin/bash
i=0
j=50
    while [ "$i" -lt "$j" ];
    	do
            sleep 300s
    		time=$(date "+%Y-%m-%d %H:%M:%S")
    		echo "net anomaly insert ${time}" >>nettime.txt
        	wondershaper veth92d2374 10000 10000
        	sleep 40s
        	wondershaper clear veth92d2374
			let "i+=1"		
    done

