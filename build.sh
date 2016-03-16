sudo apt-get update
sudo apt-get install git cmake build-essential libQt4-dev libusb-1.0-0-dev python-dev libQt4-dev
git submodule init
git submodule update

cd build 
cmake -DENABLE_DISPMANX=OFF -DENABLE_SPIDEV=OFF -DENABLE_X11=ON ..
make