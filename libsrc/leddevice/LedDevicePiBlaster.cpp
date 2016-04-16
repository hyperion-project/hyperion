
// STL includes
#include <cerrno>
#include <cstring>

// jsoncpp includes
#include <json/json.h>

// QT includes
#include <QFile>

// Local LedDevice includes
#include "LedDevicePiBlaster.h"

LedDevicePiBlaster::LedDevicePiBlaster(const std::string & deviceName, const Json::Value & gpioMapping) :
	_deviceName(deviceName),
	_fid(nullptr)
{

// { "gpio" : 4, "ledindex" : 0, "ledcolor" : "r" },

// initialise the mapping tables
// -1 is invalid
// z is also meaningless
#define TABLE_SZ sizeof(_gpio_to_led)/sizeof(_gpio_to_led[0])
	for (unsigned int i=0; i < (sizeof(_gpio_to_led)/sizeof(_gpio_to_led[0])); i++ )
	{
		_gpio_to_led[i] = -1;
		_gpio_to_color[i] = 'z';
//		printf (".");
	}

// walk through the json config and populate the mapping tables
	for (const Json::Value& gpioMap : gpioMapping)
	{
		const int gpio = gpioMap["gpio"].asInt();
		const int ledindex = gpioMap.get("ledindex",-1).asInt();
		const std::string ledcolor = gpioMap.get("ledcolor","z").asString();
//		printf ("got gpio %d ledindex %d color %c\n", gpio,ledindex, ledcolor[0]);
		_gpio_to_led[gpio] = ledindex;
		_gpio_to_color[gpio] = ledcolor[0]; // 1st char of string
	}
}

LedDevicePiBlaster::~LedDevicePiBlaster()
{
	// Close the device (if it is opened)
	if (_fid != nullptr)
	{
		fclose(_fid);
		_fid = nullptr;
	}
}

int LedDevicePiBlaster::open(bool report)
{
	if (_fid != nullptr)
	{
		// The file pointer is already open
		if (report)
		{
			std::cerr << "Attempt to open allready opened device (" << _deviceName << ")" << std::endl;
		}
		return -1;
	}

	if (!QFile::exists(_deviceName.c_str()))
	{
		if (report)
		{
			std::cerr << "The device(" << _deviceName << ") does not yet exist, can not connect (yet)." << std::endl;
		}
		return -1;
	}

	_fid = fopen(_deviceName.c_str(), "w");
	if (_fid == nullptr)
	{
		if (report)
		{
			std::cerr << "Failed to open device (" << _deviceName << "). Error message: " << strerror(errno) << std::endl;
		}
		return -1;
	}

	std::cout << "Connect to device(" << _deviceName << ")" << std::endl;

	return 0;
}

//Channel number    GPIO number   Pin in P1 header
//      0               4             P1-7
//      1              17             P1-11
//      2              18             P1-12
//      4              27             P1-??
//      5              21             P1-13
//      6              22             P1-15
//      7              23             P1-16
//      8              24             P1-18
//      9              25             P1-22
int LedDevicePiBlaster::write(const std::vector<ColorRgb> & ledValues)
{
	// Attempt to open if not yet opened
	if (_fid == nullptr && open(false) < 0)
	{
		return -1;
	}

	std::vector<int> iPins = {4, 17, 18, 27, 21, 22, 23, 24, 25};

	int valueIdx = -1;
	for (unsigned iPin=0; iPin<iPins.size(); ++iPin)
	{
		valueIdx = _gpio_to_led[ iPins[iPin] ];
		if ( (valueIdx >= 0) && (valueIdx < (signed) ledValues.size()) ) 
		{
			double pwmDutyCycle = 0.0;
//			printf ("iPin %d valueIdx %d color %c\n", iPin, valueIdx, _gpio_to_color[ iPins[iPin] ] ) ;
			switch (_gpio_to_color[ iPins[iPin] ]) 
			{
			case 'r':
				pwmDutyCycle = ledValues[valueIdx].red / 255.0;
				break;
			case 'g':
				pwmDutyCycle = ledValues[valueIdx].green / 255.0;
				break;
			case 'b':
				pwmDutyCycle = ledValues[valueIdx].blue / 255.0;
				break;
			case 'w':
				pwmDutyCycle = ledValues[valueIdx].red;
				pwmDutyCycle += ledValues[valueIdx].green;
				pwmDutyCycle += ledValues[valueIdx].blue;
				pwmDutyCycle /= (3.0*255.0);
				break;
			default:
				continue;
			}

			fprintf(_fid, "%i=%f\n", iPins[iPin], pwmDutyCycle);
			fflush(_fid);
		}
	}

	return 0;
}

int LedDevicePiBlaster::switchOff()
{
	// Attempt to open if not yet opened
	if (_fid == nullptr && open(false) < 0)
	{
		return -1;
	}

	std::vector<int> iPins = {4, 17, 18, 27, 21, 22, 23, 24, 25};

	int valueIdx = -1;
	for (unsigned iPin=0; iPin<iPins.size(); ++iPin)
	{
		valueIdx = _gpio_to_led[ iPins[iPin] ];
		if (valueIdx >= 0)
		{
			fprintf(_fid, "%i=%f\n", iPins[iPin], 0.0);
			fflush(_fid);
		}
	}

	return 0;
}
