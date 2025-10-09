echo "Telementry [y/n]"
read telementry
if [ "${telementry,,}" == "y" ]; then
    # Get login details
    logged_in=0
    while [ $logged_in -eq 0 ]; do
        echo "Please enter login details."
        read -p "Server IP: " serverip
        read -p "Rtmp server ip: " rtmp_ip
        read -p "Username: " username
        read -p "Password: " password

        # Check login
        login_status=$(curl -s -o /dev/null -w "%{http_code}" -H "username: $username" -H "passwd: $password" "$serverip/api/login")
        echo $login_status
        if [ $login_status == "200" ]; then
            echo "Setting environment variables"

            # Login successful.
            logged_in=1
            # Configure login for software.
            export SERVERIP=$serverip
            export RTMPSERVERIP=$rtmp_ip
            export SERVERUSERNAME=$username
            export SERVERPASSWD=$password
            break
        fi
    done
fi

# Get race number.
read -p "Enter race number: " racenum
export RACENUM="$racenum"

read -p "Enter wlan country: " wlan_country

raspi-config nonint do_spi 1
raspi-config nonint do_serial_hw 1
raspi-config nonint do_wifi_country $wlan_country

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
mv ../scripts/raspberrypiupdate.sh ~/update.sh
cd ../../
rm -rf gokart

echo "Gokart service setup."
sleep 1
reboot