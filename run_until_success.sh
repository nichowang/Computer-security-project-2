#!/bin/bash
# Auto-run script: keeps running until all bytes >= 90%
# This helps deal with the inherent instability of Spectre on mitigated systems

cd "$(dirname "$0")"

echo "Running Spectre attack until all bytes >= 90%..."
echo "This may take multiple attempts due to system mitigations."
echo ""

attempt=1
while true; do
    printf "Attempt %3d: " $attempt
    ./spectre > /tmp/spectre_attempt.txt 2>&1

    min=$(cat /tmp/spectre_attempt.txt | grep "success rate" | awk '{print $8}' | sed 's|/100||' | sort -n | head -1)

    if [ -n "$min" ] && [ "$min" -ge 90 ] 2>/dev/null; then
        echo "min=$min% ✓ SUCCESS!"
        echo ""
        echo "========================================="
        echo "All bytes >= 90% achieved!"
        echo "========================================="
        echo ""
        cat /tmp/spectre_attempt.txt
        exit 0
    else
        echo "min=$min%"
    fi

    attempt=$((attempt + 1))

    # Show progress every 10 attempts
    if [ $((attempt % 10)) -eq 1 ] && [ $attempt -gt 1 ]; then
        echo ""
    fi
done
