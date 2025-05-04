#!/usr/bin/bash

# Check dependencies.
if ! dpkg -l | grep -q g++; then
    echo "Build tools missing, installing."
    # Install build-essential (includes g++)
    apt update
    apt install build-essential -y
fi

if ! dpkg -l | grep -q wiringPi; then
    echo "Dependency wiringPi missing, installing."
    # Download wiringPi lib. We want to use 3.14.
    wget https://github.com/WiringPi/WiringPi/releases/download/3.14/wiringpi_3.14_arm64.deb
    # Install
    apt install ./wiringpi_3.14_arm64.deb -y
fi

if ! dpkg -l | grep -q libcurl4-openssl-dev; then
    echo "Dependency libcurl4-openssl-dev missing, installing."
    # Install libcurl library
    apt update
    apt install libcurl4-openssl-dev -y
fi

# Compile.
g++ main.cpp -lwiringPi $(pkg-config --cflags --libs libcurl) -o main
chmod a+x main