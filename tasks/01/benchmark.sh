#!/bin/sh

for testType in a A b B c C ; do
    times=()
    for i in $(seq 1 10) ; do
        times+=($(./a.out $testType))
    done
    #echo $testType "${times[@]}"
    # calc. stats:
    sum=0
    for val in ${times[@]} ; do
         sum=`expr $sum + $val`
    done
    avrg=`expr $sum / 10`
    stdDev=0
    for val in ${times[@]} ; do
         stdDev=`python -c "print('%d'%($stdDev + abs($avrg - $val)**2))"`
    done
    stdDev=`python -c "print('%d'%(($stdDev/10)**.5))"`
    precision=`python -c "print('%.2f'%(100*$stdDev / $avrg))"`
    echo "($avrg +/- $stdDev), rel.err.=$precision%"
done

