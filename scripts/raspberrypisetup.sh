# Check dependencies.
if ! $(apt list --installed | grep -q git); then
    echo "Dependency git missing, installing."
    # Install libcurl library
    apt update
    apt install git -y
fi

if ! $(apt list --installed | grep -q ffmpeg); then
    echo "Ffmpeg missing, installing."
    # Install build-essential (includes g++)
    apt update
    apt install ffmpeg -y
fi

echo "Cloning github repository."
git clone https://github.com/Sheep26/gokart.git

# Configure I2C.
raspi-config nonint do_i2c 1
raspi-config nonint do_boot_behaviour B2

cd gokart/service

# Compile executable.
echo "Compiling executable."
chmod a+x ./compile.sh
./compile.sh

# Create service.
echo "Creating service."
mv main /usr/bin/gokart-main
mv gokart.service /etc/systemd/system/gokart.service

# Enable service.
systemctl enable gokart.service
systemctl start gokart.service

echo "Cleaning up."
cd ../../
rm -rf gokart