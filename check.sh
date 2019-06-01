#!/bin/bash
while true
do
    dmesg|tail -n 40 >check.out
    sleep 1
done