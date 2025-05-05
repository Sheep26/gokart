#!/usr/bin/bash

# Check dependencies.
if ! [$(apt list | grep -q git)]; then
    echo "Dependency git missing, installing."
    # Install libcurl library
    apt update
    apt install git -y
fi

if ! [$(apt list | grep -q g++)]; then
    echo "Build tools missing, installing."
    # Install build-essential (includes g++)
    apt update
    apt install build-essential -y
fi

if ! [$(apt list | grep -q wiringPi)]; then
    echo "Dependency wiringPi missing, installing."
    git clone https://github.com/WiringPi/WiringPi.git
    # Install
    cd WiringPi
    ./build debian

    mv debian-template/wiringpi_* .
    apt install ./wiringpi_*

    # Clean up
    cd ../
    rm -rf WiringPi
fi

if ! [$(apt list | grep -q libcurl4-openssl-dev)]; then
    echo "Dependency libcurl4-openssl-dev missing, installing."
    # Install libcurl library
    apt update
    apt install libcurl4-openssl-dev -y
fi

# Compile.
g++ main.cpp -l wiringPi $(pkg-config --cflags --libs libcurl) -o main
chmod a+x main