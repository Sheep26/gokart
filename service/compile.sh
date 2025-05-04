# Check dependencies.
if ! [-z $(apt list | grep wiringPi)]; then
    echo "Dependency wiringPi missing, installing."
    # Download wiringPi lib. We want to use 3.14.
    wget https://github.com/WiringPi/WiringPi/releases/download/3.14/wiringpi_3.14_arm64.deb
    # Install
    apt install ./wiringpi_3.14_arm64.deb -y
fi

if ! [-z $(apt list | grep libcurl4-openssl-dev)]; then
    echo "Dependency libcurl4-openssl-dev missing, installing."
    # Install libcurl library
    apt update
    apt install libcurl4-openssl-dev -y
fi

# Compile.
g++ main.cpp -l wiringPi $(pkg-config --cflags --libs libcurl) -o main