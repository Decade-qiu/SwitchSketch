#!/bin/bash

# Create result directory if it doesn't exist
mkdir -p ./result

# Delete old files
rm -f ./result/*

# Run Python scripts
python3 WavingSketch.py

python3 HeavyGuardian_offline.py

python3 DHS128_offline.py

python3 SwitchSketch128_offline_with_double_check.py
