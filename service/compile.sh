#!/usr/bin/bash
if [[ "$*" != *-u* ]]; then
    apt update
    #apt upgrade -y

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

    if ! $(apt list --installed | grep -q libsndfile1-dev); then
        echo "libsndfile1-dev Missing, installing."

        apt install libsndfile1-dev -y
    fi

    if ! $(apt list --installed | grep -q libasio-dev); then
        echo "libasio-dev Missing, installing."

        apt install libasio-dev -y
    fi

    if ! $(apt list --installed | grep -q crow); then
        echo "crow Missing, installing."

        wget https://github.com/CrowCpp/Crow/releases/download/v1.2.1.2/Crow-1.2.1-Linux.deb
        apt install ./Crow-1.2.1-Linux.deb -y
        rm Crow-1.2.1-Linux.deb
    fi
fi

# -lasound

# Compile.
g++ *.cpp -O2 -vv -fpermissive -lwiringPi -lfmt $(pkg-config --cflags --libs libcurl) -lbluetooth -fpermissive -lpthread -o main
chmod a+x main