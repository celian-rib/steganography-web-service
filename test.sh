#!/bin/bash

TESTSTR="Bonsoir"

echo "Starting test"

echo "Encoding image"
./main -encode otter.bmp $TESTSTR otter.bmp

echo "Encoding image"
./main -decode otter.bmp > test-output.txt

if ! grep -q $TESTSTR "test-output.txt"; then
    echo "------------------------"
    echo "Error in test"
    echo "------------------------"
    exit 1
else
    echo "------------------------"
    echo "TEST PASSED"
    echo "------------------------"
    exit 0
fi
