#include <iostream>
// BlackBorders includes
#include <blackborder/BlackBorderDetector.h>

using namespace hyperion;

//BlackBorderDetector::BlackBorderDetector(uint8_t blackborderThreshold) :
BlackBorderDetector::BlackBorderDetector(double threshold) :
	_threshold(threshold)
{
	_calculateThreshold(_threshold);
	// empty
}

void BlackBorderDetector::_calculateThreshold(double blackborderThreshold)
{
	int threshold = int(std::ceil(blackborderThreshold * 255));
	if (threshold < 0)
		threshold = 0;
	else if (threshold > 255)
		threshold = 255;
	_blackborderThreshold = uint8_t(threshold);

	std::cout << "Black border threshold set to " << blackborderThreshold << " (" << int(_blackborderThreshold) << ")" << std::endl;
}