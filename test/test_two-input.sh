#!/bin/bash

while read row; do
    s1=`echo ${row} | fold -s1 | head -n1`
    if [ ${s1} != '#' ]; then
        x=`echo ${row} | cut -d , -f 1`
        y=`echo ${row} | cut -d , -f 2`
        ex=`echo ${row} | cut -d , -f 3`
        
        ./run_two-input.sh ${x} ${y} ${ex}
    fi
done < testcase_two-input.csv
