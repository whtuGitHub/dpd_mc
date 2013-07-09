#!/bin/bash

echo "Job $1" > output"$1".log

for ((window=$2;window<$3;window++)); do

    echo "Window $window" >> output"$1".log
    sed -i "s/.*iQ_init.*/$window\t\t\tiQ_init/g" dpd.inp

    # Delete existing random seed, generate a new one and add it to input file
    sed -i '$d' dpd.inp
    ./seed.exe >> dpd.inp

    ./dpd.run >> output"$1".log

    # Move output files
    mv PQ.dat "$4"/PQ/PQ"$window".dat
    mv Q.dat "$4"/Q/Q"$window".dat
    mv energy.dat "$4"/energy/energy"$window".dat
    mv re.dat "$4"/re/re"$window".dat
    mv rg.dat "$4"/rg/rg"$window".dat
    cp dpd.inp "$4"/input/dpd"$window".inp
done

cp output"$1".log "$4"/output