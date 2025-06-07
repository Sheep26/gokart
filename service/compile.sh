#!/usr/bin/bash
apt update
apt upgrade

# Check dependencies.
if ! $(apt list --installed | grep -q git); then
    echo "Dependency git missing, installing."

    # Install libcurl library
    apt install git -y
fi

if ! $(apt list --installed | grep -q g++); then
    echo "Build tools missing, installing."

    # Install build-essential (includes g++)
    apt install build-essential -y
fi

if ! $(apt list --installed | grep -q libfmt-dev); then
    echo "Dependency libfmt-dev missing, installing."

    # Install libfmt-dev
    apt install libfmt-dev -y
fi

if ! $(apt list --installed | grep -q wiringpi); then
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

if ! $(apt list --installed | grep -q pkg-config); then
    echo "Dependency pkg-config missing, installing."

    # Install libcurl library
    apt install pkg-config -y
fi

if ! $(apt list --installed | grep -q libcurl4-openssl-dev); then
    echo "Dependency libcurl4-openssl-dev missing, installing."

    # Install libcurl library
    apt install libcurl4-openssl-dev -y
fi

if ! $(apt list --installed | grep -q libbluetooth-dev); then
    echo "Dependency libbluetooth-dev missing, installing."

    # Install libcurl library
    apt install libbluetooth-dev -y
fi

#if ! $(apt list --installed | grep -q libasound2-dev); then
#    echo "Dependency libasound2-dev missing, installing"

    # Install the library.
#    apt install libasound2-dev -y
#fi

# -lasound

# Compile.
g++ main.cpp networking.cpp OledScreen.cpp -lwiringPi -lfmt $(pkg-config --cflags --libs libcurl) -lpthread -o main
chmod a+x main