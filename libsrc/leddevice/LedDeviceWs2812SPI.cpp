
// STL includes
#include <cstring>
#include <cstdio>
#include <iostream>

// Linux includes
#include <fcntl.h>
#include <sys/ioctl.h>

// hyperion local includes
#include "LedDeviceWs2812SPI.h"

LedDeviceWs2812SPI::LedDeviceWs2812SPI(const std::string& outputDevice, const unsigned baudrate) :
	LedSpiDevice(outputDevice, baudrate, 500000),
	mLedCount(0)
{
	// empty
}

LedDeviceWs2812SPI::LedDeviceWs2812SPI(const std::string& outputDevice, const unsigned baudrate, const unsigned latchTime) :
	LedSpiDevice(outputDevice, baudrate, latchTime),
	mLedCount(0)
{
	// empty
}

int LedDeviceWs2812SPI::write(const std::vector<ColorRgb> &ledValues)
{
	uint8_t bitpair_to_byte[] = {
		0b10001000,
		0b10001100,
		0b11001000,
		0b11001100,
	};

	mLedCount = ledValues.size();
// 3 colours, 4 spi bytes per colour + 3 frame end latch bytes
	unsigned spi_size = mLedCount * 3 * 4 + 3;
	if(_spiBuffer.size() != spi_size){
                _spiBuffer.resize(spi_size, 0x00);
	}

	unsigned spi_ptr = 0;
        for (int iLed=0; iLed < mLedCount; ++iLed) {
                const ColorRgb& rgb = ledValues[iLed];

		unsigned a = rgb.red;
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b11000000)>>6];
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b00110000)>>4];
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b00001100)>>2];
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b00000011)>>0];

		a = rgb.green;
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b11000000)>>6];
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b00110000)>>4];
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b00001100)>>2];
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b00000011)>>0];

		a = rgb.blue;
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b11000000)>>6];
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b00110000)>>4];
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b00001100)>>2];
		_spiBuffer[spi_ptr++] = bitpair_to_byte[(a&0b00000011)>>0];
        }


	return writeBytes(spi_size, _spiBuffer.data());
}

int LedDeviceWs2812SPI::switchOff()
{
	return write(std::vector<ColorRgb>(mLedCount, ColorRgb{0,0,0}));
}
