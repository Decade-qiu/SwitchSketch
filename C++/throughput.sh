#!/bin/bash

# Delete old files
rm -f ./*.txt

# Compile and run DHS
cd ./DHS
g++ DHS.cpp -O3 -o DHS
./DHS

# Compile and run HeavyGuardian
cd ../HeavyGuardian
g++ HeavyGuardian.cpp -O3 -o HeavyGuardian
./HeavyGuardian

# Compile and run WavingSketch
cd ../WavingSketch
g++ WavingSketch.cpp -O3 -o WavingSketch
./WavingSketch

# Compile and run SwitchSketch
cd ../SwitchSketch
g++ SwitchSketch-128bit-offline-withoutDoubleCheck.cpp -O3 -o SwitchSketch
./SwitchSketch