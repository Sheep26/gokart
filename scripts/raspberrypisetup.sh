# Check if ffmpeg installed
if ! dpkg -l | grep -q ffmpeg; then
    echo "Ffmpeg missing, installing."
    # Install build-essential (includes g++)
    apt update
    apt install ffmpeg -y
fi

echo "Cloning github repository."
git clone https://github.com/Sheep26/gokart

cd gokart/service

# Compile executable.
echo "Compiling executable."
chmod a+x ./compile.sh
./compile.sh

# Create service.
echo "Creating service."
mv main /usr/bin/gokart-main
mv gokart.service /etc/systemd/system/gokart.service
cd ../

# Enable service.
systemctl enable gokart.service
systemctl start gokart.service

echo "Cleaning up."
rm -rf gokart