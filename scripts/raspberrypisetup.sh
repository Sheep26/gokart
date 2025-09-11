echo "Telementry [y/n]"
read telementry
if [ "${telementry,,}" == "y" ]; then
    # Get login details
    logged_in=0
    while [ "$logged_in" -eq 0 ]; do
        echo "Please enter login details."
        echo "Server ip: "
        read serverip
        echo "Rtmp server ip"
        read rtmp_ip
        echo "Username: "
        read username
        echo "Password: "
        read password

        # Check login
        login_status=$(curl -s -o /dev/null -w "%{http_code}" -H "username: $username" -H "passwd: $password" "$serverip/api/login")
        if [ "$login_status" == "200" ]; then
            # Login successful.
            logged_in=1
            # Configure login for software.
            export SERVERIP="$serverip"
            export RTMPSERVERIP="$rtmp_ip"
            export SERVERUSERNAME="$username"
            export SERVERPASSWD="$password"
            break
        fi
    done
fi

# Get race number.
echo "Enter race number"
read racenum
export RACENUM="$racenum"

raspi-config nonint do_spi 1

#echo "Cloning github repository."
#git clone https://github.com/Sheep26/gokart.git

cd ../service

# Compile executable.
echo "Compiling executable."
chmod a+x ./compile.sh
./compile.sh

# Create service.
echo "Creating service."
mv main /usr/bin/gokart-main
mv gokart.service /etc/systemd/system/gokart.service

# Enable services.
systemctl enable bluetooth
systemctl start bluetooth
systemctl enable gokart.service
systemctl start gokart.service

echo "Cleaning up."
cd ../../
rm -rf gokart

echo "Gokart service setup Rebooting raspberry pi."
reboot