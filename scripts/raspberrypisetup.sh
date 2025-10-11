rm /root/.server_env
read -p "Telementry [y/n]" telementry
echo "export TELEMENTRY=${telementry,,}" > /root/.server_env
if [ "${telementry,,}" == "y" ]; then
    # Get hotspot details
    echo "Please enter wifi hotspot login details."
    
    read -p "SSID: " wifissid
    read -p "PASSWD: " wifipasswd

    echo "export WLANSSID=$wifissid" >> /root/.server_env
    echo "export WLANPASSWD=$wifipasswd" >> /root/.server_env

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
            # Login successful.
            logged_in=1
            # Configure login for software.
            echo "export SERVERIP=$serverip" >> /root/.server_env
            echo "export RTMPSERVERIP=$rtmp_ip" >> /root/.server_env
            echo "export SERVERUSERNAME=$username" >> /root/.server_env
            echo "export SERVERPASSWD=$password" >> /root/.server_env
            break
        fi
    done
fi

# Get race number.
read -p "Enter race number: " racenum
export RACENUM="$racenum"
echo "export RACENUM=$racenum" >> /root/.server_env

#read -p "Enter wlan country: " wlan_country

#raspi-config nonint do_spi 1
#raspi-config nonint do_serial_hw 1
#raspi-config nonint do_wifi_country $wlan_country

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

echo "source /root/.server_env" > /root/start.sh
echo "/usr/bin/gokart-main" >> /root/start.sh

mv gokart.service /etc/systemd/system/gokart.service

# Enable services.
systemctl enable gokart.service
systemctl start gokart.service

echo "Cleaning up."
mv ../scripts/raspberrypiupdate.sh /root/update.sh
cd ../../
rm -rf gokart

echo "Gokart service setup."
sleep 1
#reboot