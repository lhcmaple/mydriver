#!/bin/bash
>check.out
while true
do
    if isavail /dev/scullpipe0 r
    then
        get /dev/scullpipe0 10 0 0 >>check.out
    fi
    sleep 1
done