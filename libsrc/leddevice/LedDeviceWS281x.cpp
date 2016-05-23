#include <iostream>

#include "LedDeviceWS281x.h"

// Constructor
LedDeviceWS281x::LedDeviceWS281x(const int gpio, const int leds, const uint32_t freq, const int dmanum, const int pwmchannel, const int invert, const int rgbw, const std::string& whiteAlgorithm)
{
	_whiteAlgorithm = whiteAlgorithm;
std::cout << "whiteAlgorithm :" << whiteAlgorithm << ":\n";
	initialized = false;
	led_string.freq = freq;
	led_string.dmanum = dmanum;
	if (pwmchannel != 0 && pwmchannel != 1) {
		std::cout << "WS281x: invalid PWM channel; must be 0 or 1." << std::endl;
		throw -1;
	}
	chan = pwmchannel;
	led_string.channel[chan].gpionum = gpio;
	led_string.channel[chan].invert = invert;
	led_string.channel[chan].count = leds;
	led_string.channel[chan].brightness = 255;
	if (rgbw == 1) {
		led_string.channel[chan].strip_type = SK6812_STRIP_GRBW;
	} else {
		led_string.channel[chan].strip_type = WS2811_STRIP_RGB;
	}

	led_string.channel[!chan].gpionum = 0;
	led_string.channel[!chan].invert = invert;
	led_string.channel[!chan].count = 0;
	led_string.channel[!chan].brightness = 0;
	led_string.channel[!chan].strip_type = WS2811_STRIP_RGB;
	if (ws2811_init(&led_string) < 0) {
		std::cout << "Unable to initialize ws281x library." << std::endl;
		throw -1;
	}
	initialized = true;
}

// Send new values down the LED chain
int LedDeviceWS281x::write(const std::vector<ColorRgb> &ledValues)
{
	if (!initialized)
		return -1;

	int idx = 0;
	for (const ColorRgb& color : ledValues)
	{
		if (idx >= led_string.channel[chan].count)
			break;
		_temp_rgbw.red = color.red;
		_temp_rgbw.green = color.green;
		_temp_rgbw.blue = color.blue;
		_temp_rgbw.white = 0;

		Rgb_to_Rgbw(color, &_temp_rgbw, _whiteAlgorithm);

		unsigned white = 0;
		unsigned red = color.red;
		unsigned green = color.green;
		unsigned blue = color.blue;
// dodgy colour correction
		if (led_string.channel[chan].strip_type == SK6812_STRIP_GRBW) {
// pretty dumb - assumes its linear (but its more like exponential
			if (_whiteAlgorithm == "subtract_minimum") {
				white = std::min(red, green);
				white = std::min(white, blue);
				red -= white;
				green -= white;
				blue -= white;
			} 
			else if (_whiteAlgorithm == "sub_min_warm_adjust") {
				white = std::min(red, green);
				white = std::min(white, blue);
				red -= white;
				green -= white;
				blue -= white;
			}
			else if ( (_whiteAlgorithm == "") || (_whiteAlgorithm == "white_off") ) {
				white = 0;
			}
		}

		led_string.channel[chan].leds[idx++] = 
			((uint32_t)white << 24) + ((uint32_t)red << 16) + ((uint32_t)green << 8) + blue;
//		led_string.channel[chan].leds[idx++] = 
//			((uint32_t)white << 24) + ((uint32_t)red << 16) + ((uint32_t)green << 8) + blue;
	}
	while (idx < led_string.channel[chan].count)
		led_string.channel[chan].leds[idx++] = 0;

	if (ws2811_render(&led_string))
		return -1;

	return 0;
}

// Turn off the LEDs by sending 000000's
// TODO Allow optional power switch out another gpio, if this code handles it can
// make it more likely we don't accidentally drive data into an off strip
int LedDeviceWS281x::switchOff()
{
	if (!initialized)
		return -1;

	int idx = 0;
	while (idx < led_string.channel[chan].count)
		led_string.channel[chan].leds[idx++] = 0;

	if (ws2811_render(&led_string))
		return -1;

	return 0;
}

// Destructor
LedDeviceWS281x::~LedDeviceWS281x()
{
	if (initialized)
	{
		std::cout << "Shutdown WS281x PWM and DMA channel" << std::endl;
		ws2811_fini(&led_string);
	}
	initialized = false;
}
