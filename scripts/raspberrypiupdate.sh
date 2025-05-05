echo "Cloning github repository."
git clone https://github.com/Sheep26/gokart

cd gokart/service

# Compile executable.
echo "Compiling executable."
chmod a+x ./compile.sh
./compile.sh

# Update executable.
echo "Updating executable."
rm /usr/bin/gokart-main
mv main /usr/bin/gokart-main

systemctl restart gokart.service

echo "Cleaning up."
cd ../../
rm -rf gokart