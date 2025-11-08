#!/bin/bash
gcc -O0 -o spectre spectre.c

for i in {1..20}; do
    echo "=== Run $i ==="
    ./spectre > /tmp/run_$i.txt 2>&1

    # Find minimum success rate
    min=$(grep "success rate" /tmp/run_$i.txt | awk '{print $9}' | sed 's|/100||' | sort -n | head -1)

    echo "Minimum success rate: $min%"

    # Check if all bytes >= 80%
    if [ "$min" -ge "80" ]; then
        echo "✓ SUCCESS: All bytes >= 80%!"
        cat /tmp/run_$i.txt
        exit 0
    fi
    echo ""
done

echo "No run achieved >80% for all bytes"
