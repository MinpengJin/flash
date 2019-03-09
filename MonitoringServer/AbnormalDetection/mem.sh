#!/bin/bash
i=0
j=25
    while [ "$i" -lt "$j" ];
    	do
    		sleep 300s
    		time=$(date "+%Y-%m-%d %H:%M:%S")
    		echo "mem anomaly insert ${time}" >>mem.txt
        	./mem
			let "i+=1"			
    done

