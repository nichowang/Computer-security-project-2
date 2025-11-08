#!/bin/bash
echo "=== Spectre Attack Test Script ==="
echo "Compiling..."
gcc -o spectre_new spectre_new.c -O0 -msse2 -march=native -w

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Compilation successful!"
echo ""
echo "Running attack (this will take 5-10 minutes)..."
echo "Press Ctrl+C to stop if needed."
echo ""

./spectre_new

echo ""
echo "=== Test Complete ==="
