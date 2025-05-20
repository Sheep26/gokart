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

if ! $(apt list --installed | grep -q libsndfile1-dev); then
    echo "libsndfile1-dev Missing, installing."

    apt install libsndfile1-dev -y
fi

echo "Telementry [y/n]"
read telementry
if [ "${telementry,,}" == "y" ]; then
    # Get login details
    logged_in=0
    while [ "$logged_in" -eq 0 ]; do
        echo "Please enter login details."
        echo "Server ip: "
        read serverip
        echo "Username: "
        read username
        echo "Password: "
        read password

        # Check login
        login_status=$(curl -s -o /dev/null -w "%{http_code}" "$serverip/api/login")
        if [ "$login_status" == "200" ]; then
            # Login successful.
            logged_in=1
            # Configure login for software.
            export SERVERIP="$serverip"
            export SERVERUSERNAME="$username"
            export SERVERPASsWD="$password"
        fi
    done
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