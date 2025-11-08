#!/bin/bash
# Spectre Attack - Auto-run until all bytes >= 85%
# Usage: ./run_spectre.sh

cd "$(dirname "$0")"

while true; do
    # Run the attack silently
    ./spectre > /tmp/spectre_result.txt 2>&1

    # Check if all bytes >= 85%
    min=$(grep "success rate" /tmp/spectre_result.txt | awk '{print $8}' | sed 's|/100||' | sort -n | head -1)

    if [ -n "$min" ] && [ "$min" -ge 85 ] 2>/dev/null; then
        # Found! Just show the result
        cat /tmp/spectre_result.txt
        exit 0
    fi
done
