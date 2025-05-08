# Check dependencies.
apt update

if ! $(apt list --installed | grep -q git); then
    echo "Dependency git missing, installing."
    # Install libcurl library
    apt install git -y
fi

if ! $(apt list --installed | grep -q ffmpeg); then
    echo "Ffmpeg missing, installing."
    # Install build-essential (includes g++)
    apt install ffmpeg -y
fi

if ! $(apt list --installed | grep -q alsa-utils); then
    echo "Dependency alsa-utils missing, installing."
    # Install libcurl library
    apt install alsa-utils -y
fi

echo "Cloning github repository."
git clone https://github.com/Sheep26/gokart.git

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

echo "Gokart service setup Rebooting raspberry pi"
reboot