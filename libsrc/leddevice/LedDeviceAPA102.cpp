
// STL includes
#include <cstring>
#include <cstdio>
#include <iostream>
#include <algorithm>

// Linux includes
#include <fcntl.h>
#include <sys/ioctl.h>

// hyperion local includes
#include "LedDeviceAPA102.h"

LedDeviceAPA102::LedDeviceAPA102(const std::string& outputDevice, const unsigned baudrate, const unsigned ledcount) :
	LedSpiDevice(outputDevice, baudrate, 500000),
	_ledBuffer(0)
{
	_ledcount = ledcount;
}

int LedDeviceAPA102::write(const std::vector<ColorRgb> &ledValues)
{
	const unsigned int max_leds = std::max(ledValues.size(), _ledcount);
	const unsigned int startFrameSize = 4;
//	const unsigned int endFrameSize = std::max<unsigned int>(((ledValues.size() + 15) / 16), 4);
//	const unsigned int APAbufferSize = (ledValues.size() * 4) + startFrameSize + endFrameSize;
	const unsigned int endFrameSize = std::max<unsigned int>(((max_leds + 15) / 16), 4);
	const unsigned int APAbufferSize = (max_leds * 4) + startFrameSize + endFrameSize;

	printf ("ledValues.size() %d _ledcount %d max_leds %d APAbufferSize %d\n",
		ledValues.size(), _ledcount, max_leds, APAbufferSize);
	if(_ledBuffer.size() != APAbufferSize){
		_ledBuffer.resize(APAbufferSize, 0xFF);
		_ledBuffer[0] = 0x00; 
		_ledBuffer[1] = 0x00; 
		_ledBuffer[2] = 0x00; 
		_ledBuffer[3] = 0x00; 
	}
	
	for (unsigned iLed=1; iLed<=ledValues.size(); ++iLed) {
		const ColorRgb& rgb = ledValues[iLed-1];
		_ledBuffer[iLed*4]   = 0xFF;
		_ledBuffer[iLed*4+1] = rgb.red;
		_ledBuffer[iLed*4+2] = rgb.green;
		_ledBuffer[iLed*4+3] = rgb.blue;
	}

	return writeBytes(_ledBuffer.size(), _ledBuffer.data());
}

int LedDeviceAPA102::switchOff()
{
	return write(std::vector<ColorRgb>(_ledBuffer.size(), ColorRgb{0,0,0}));
}
