#!/bin/bash

while read row; do
    x=`echo ${row} | cut -d , -f 1`
    ex=`echo ${row} | cut -d , -f 2`

    ./run_one-input.sh ${x} ${ex}
done < testcase_one-input.csv
