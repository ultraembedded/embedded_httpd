# Toolchain (https://github.com/esp8266/esp8266-wiki/wiki/Toolchain)
```
sudo mkdir -p /opt/Espressif
sudo chown -R $USER /opt/Espressif
cd /opt/Espressif
```

# Install crosstool-NG (toolchain)
```
git clone --depth 1 -b lx106 git://github.com/jcmvbkbc/crosstool-NG.git 
cd crosstool-NG
./bootstrap && ./configure --prefix=`pwd` && make && make install
./ct-ng xtensa-lx106-elf
./ct-ng build
```

# Add toolchain to path
```
echo "export PATH=$PWD/builds/xtensa-lx106-elf/bin:\$PATH" | sudo tee -a /root/.bashrc
echo "export PATH=$PWD/builds/xtensa-lx106-elf/bin:\$PATH" >> ~/.bashrc
```

# Get latest SDK
```
cd /opt/Espressif
wget -O esp_iot_sdk_v1.2.0_15_07_03.zip https://github.com/esp8266/esp8266-wiki/raw/master/sdk/esp_iot_sdk_v1.2.0_15_07_03.zip
unzip esp_iot_sdk_v1.2.0_15_07_03.zip
ln -s esp_iot_sdk_v1.2.0 ESP8266_SDK
```

# Patch SDK
```
cd /opt/Espressif/ESP8266_SDK
wget -O lib/libc.a https://github.com/esp8266/esp8266-wiki/raw/master/libs/libc.a
wget -O lib/libhal.a https://github.com/esp8266/esp8266-wiki/raw/master/libs/libhal.a
wget -O include.tgz https://github.com/esp8266/esp8266-wiki/raw/master/include.tgz
tar -xvzf include.tgz
```

# Build webserver
* Edit user/user_config.h to match SSID and password for your WIFI details.
* Edit Makefile ESPPORT to select correct /dev/ttyUSBx interface.
* Type 'make', then 'make flash'.
