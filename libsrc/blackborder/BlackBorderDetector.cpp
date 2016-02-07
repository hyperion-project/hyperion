#include <iostream>
// BlackBorders includes
#include <blackborder/BlackBorderDetector.h>

using namespace hyperion;

BlackBorderDetector::BlackBorderDetector(double threshold) :
	_blackborderThreshold(calculateThreshold(threshold))
{
	// empty
}

uint8_t BlackBorderDetector::calculateThreshold(double threshold)
{
	int rgbThreshold = int(std::ceil(threshold * 255));
	if (rgbThreshold < 0)
		rgbThreshold = 0;
	else if (rgbThreshold > 255)
		rgbThreshold = 255;

	uint8_t blackborderThreshold = uint8_t(rgbThreshold);

	std::cout << "Black border threshold set to " << threshold << " (" << int(blackborderThreshold) << ")" << std::endl;

	return blackborderThreshold;
}
