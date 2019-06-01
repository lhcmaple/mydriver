#!/bin/bash
>a.c
>b.c
while read -e line
do
    echo "$line" >/dev/scullpipe0
    echo "$line" >>a.c
    cat /dev/scullpipe0 >>b.c
done <scullpipe.c