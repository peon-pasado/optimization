#!/bin/bash

make

directory="Falkenauer_CSP/Falkenauer_T/"

sum=0
counter=0

for file in ${directory}Falkenauer_t60_{00..19}.txt; do
    if [ -f "$file" ]; then
        counter=$((counter + 1))
        
        start_time=$(date +%s.%N)
        ./main < "$file"
        end_time=$(date +%s.%N)
        
        execution_time=$(echo "$end_time - $start_time" | bc)
        sum=$(echo "$sum + $execution_time" | bc)
    fi
done

if [ "$counter" -gt 0 ]; then
    average=$(echo "scale=2; $sum / $counter" | bc)
    echo "The average execution time is: $average seconds"
else
    echo "No files found to calculate the average time."
fi
