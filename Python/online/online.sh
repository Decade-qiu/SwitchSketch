#!/bin/bash

# Create result directory if it doesn't exist
mkdir -p ./result

# Delete all files in the result directory
rm -f ./result/*

# Run Python scripts
python3 WavingSketch.py
python3 HeavyGuardian_online.py
python3 DHS128_online.py
python3 SwitchSketch128_online.py
