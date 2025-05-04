echo "Cloning github repository."
git clone https://github.com/Sheep26/gokart

cd gokart/service

# Compile executable.
echo "Compiling executable."
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