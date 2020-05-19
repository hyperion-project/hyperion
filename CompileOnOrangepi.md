# Install on Armbian Orange Pi Zero
>Tested on Armbian_19.11.3_Orangepizero_bionic_current_5.3.9

Following Steps can be used to build and install on Arbian on Orange Pi
1. First update and upgrade Arbian and make sure you are connected to Internet
```sh
$ sudo apt update & apt upgrade
```
>You may reboot after this 
2. Install the necessary softwares required for build 
```sh
$ sudo apt-get install git cmake build-essential libqt4-dev libusb-1.0-0-dev python-dev libxrender-dev python 
```
3. Download and build the hyperion using the following command
```sh
$ export HYPERION_DIR="hyperion"
$ git clone --recursive https://github.com/tvdzwan/hyperion.git "$HYPERION_DIR"
$ mkdir "$HYPERION_DIR/build"
$ cd "$HYPERION_DIR/build"
$ cmake -DENABLE_DISPMANX=OFF -DENABLE_OPENCV=OFF -DENABLE_SPIDEV=ON -DENABLE_X11=OFF -DCMAKE_BUILD_TYPE=Release -Wno-dev ..
$ make -j $(nproc)
$ strip bin/* 
```
4. Install manually hyperion using the following command
```sh
$ sudo cp ./bin/hyperion-remote /usr/bin/
$ sudo cp ./bin/hyperiond /usr/bin/
$ sudo cp ./bin/hyperion-v4l2 /usr/bin/
```
5. Copy the effects and configurations 
```sh
$ sudo mkdir -p /usr/share/hyperion/effects && sudo cp -R ../effects/ /usr/share/hyperion/
$ sudo mkdir -p /etc/hyperion
```
6. Copy Startup scripts 
```sh
$ sudo mkdir -p /etc/init/ && sudo cp ../bin/service/hyperion.initctl.sh  /etc/init/hyperion.conf
$ sudo cp ../bin/service/hyperion.init.sh  /etc/init.d/hyperion
$ sudo cp ../bin/service/hyperion.systemd.sh  /etc/systemd/system/hyperion.service
```
7. In case Hypercon is not able to copy automatically, manually copy the copy hyperion.config.json to /etc/hyperion folder
8. Finally to make hyperion auto start during boot up, execute following commands
```sh
$ sudo systemctl enable hyperion
$ sudo systemctl start hyperion
```
> Extras, 

To debug the usb capture 
```sh
sudo apt-get install v4l-utils
```
To disable password while sudo 
Edit sudoer list by following command and add the below line bottom.
```sh
sudo visudo -f /etc/sudoers.d/90-username
```
username ALL=(ALL) NOPASSWD:ALL
>replace username with armbian user name.     
    
### To enable SPI UART and GPIO in rpi 
edit /boot/armbianEnv.txt
```sh
$ sudo nano /boot/armbianEnv.txt
```
>overlays=usbhost2 usbhost3 w1-gpio uart1 i2c0 spi-spidev
>param_w1_pin=PA20
>param_w1_pin_int_pullup=1
>param_uart1_rtscts=1
>param_spidev_spi_bus=1

example file may look like after editing:
>verbosity=1
>console=both
>overlay_prefix=sun50i-h5
>overlays=usbhost2 usbhost3 w1-gpio uart1 i2c0 spi-spidev
>rootdev=UUID=abc1d3dd-6193-4fb7-93fb-f50382b8a8b9
>rootfstype=ext4
>param_w1_pin=PA20
>param_w1_pin_int_pullup=1
>param_uart1_rtscts=1
>param_spidev_spi_bus=0
>usbstoragequirks=0x2537:0x1066:u,0x2537:0x1068:u

Note
>If you are using pin 19 and 23 for SPI for rpi pins then spi is /dev/spidev1.0

Thats it, rpi is ready for Hyperion 