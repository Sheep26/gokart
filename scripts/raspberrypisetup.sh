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

echo "Cloning github repository."
git clone https://github.com/Sheep26/gokart.git

cd gokart/service

# Compile pi fm rds.
echo "Compiling pi fm rds."
git clone https://github.com/ChristopheJacquet/PiFmRds.git
cd PiFmRds/src
make clean
make
mv ./pi_fm_rds /usr/bin/pi_fm_rds
cd ../../
rm -rf PiFmRds

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