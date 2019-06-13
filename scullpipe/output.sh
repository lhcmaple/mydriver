#!/bin/bash
count=0
while [ $count -ne 20 ]
do
    echo -n hello >/dev/scullpipe0
done

echo -n hello >/dev/scullpipe0

>check.out
while true
do
    if isavail /dev/scullpipe0 r
    then
        get /dev/scullpipe0 10 0 0 >>check.out
    fi
    sleep 1
done