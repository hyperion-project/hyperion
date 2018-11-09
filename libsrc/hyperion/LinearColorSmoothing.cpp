// Qt includes
#include <QDateTime>

#include "LinearColorSmoothing.h"

LinearColorSmoothing::LinearColorSmoothing( LedDevice * ledDevice, double ledUpdateFrequency_hz, int settlingTime_ms, unsigned updateDelay, bool continuousOutput)
	: QObject()
	, LedDevice()
	, _ledDevice(ledDevice)
	, _updateInterval(1000 / ledUpdateFrequency_hz)
	, _settlingTime(settlingTime_ms)
	, _timer()
	, _outputDelay(updateDelay)
	, _writeToLedsEnable(false)
	, _continuousOutput(continuousOutput)
{
	_timer.setSingleShot(false);
	_timer.setInterval(_updateInterval);

	connect(&_timer, SIGNAL(timeout()), this, SLOT(updateLeds()));

	std::cout << "HYPERION (CS) INFO: Created linear-smoothing(interval_ms=" << _updateInterval << ";settlingTime_ms=" << settlingTime_ms << ";updateDelay=" << _outputDelay << std::endl;
}

LinearColorSmoothing::~LinearColorSmoothing()
{
	// Make sure to switch off the underlying led-device (because switchOff is no longer forwarded)
	_ledDevice->switchOff();
	delete _ledDevice;
}

int LinearColorSmoothing::write(const std::vector<ColorRgb> &ledValues)
{
	// received a new target color
	if (_previousValues.empty())
	{
		// not initialized yet
		_targetTime = QDateTime::currentMSecsSinceEpoch() + _settlingTime;
		_targetValues = ledValues;

		_previousTime = QDateTime::currentMSecsSinceEpoch();
		_previousValues = ledValues;
	}
	else
	{
		_targetTime = QDateTime::currentMSecsSinceEpoch() + _settlingTime;
		memcpy(_targetValues.data(), ledValues.data(), ledValues.size() * sizeof(ColorRgb));
	}

	if(!_writeToLedsEnable)
	{
		_timer.start();
		_writeToLedsEnable = true;
	}

	return 0;
}

int LinearColorSmoothing::switchOff()
{
	if(_continuousOutput)
	{
		// We will keep updating the leds (but with pure-black)

		// Clear the smoothing parameters
		std::fill(_targetValues.begin(), _targetValues.end(), ColorRgb::BLACK);
		_targetTime = 0;

		// Erase the output-queue
		for (unsigned i=0; i<_outputQueue.size(); ++i)
		{
			_outputQueue.push_back(_targetValues);
			_outputQueue.pop_front();
		}

		if(!_writeToLedsEnable)
		{
			_timer.start();
			_writeToLedsEnable = true;
		}
	}
	else
	{
		_ledDevice->switchOff();

		if(_writeToLedsEnable)
		{
			_timer.stop();
			_writeToLedsEnable = false;
		}
	}

	return 0;
}

void LinearColorSmoothing::updateLeds()
{
	int64_t now = QDateTime::currentMSecsSinceEpoch();
	int deltaTime = _targetTime - now;

	if (deltaTime < 0)
	{
		memcpy(_previousValues.data(), _targetValues.data(), _targetValues.size() * sizeof(ColorRgb));

		_previousTime = now;

		_writeToLedsEnable = _continuousOutput;
		if(!_writeToLedsEnable)
		{
			_timer.stop();
		}

		queueColors(_previousValues);
	}
	else
	{
		float k = 1.0f - 1.0f * deltaTime / (_targetTime - _previousTime);

		int reddif = 0, greendif = 0, bluedif = 0;

		for (size_t i = 0; i < _previousValues.size(); ++i)
		{
			ColorRgb & prev = _previousValues[i];
			ColorRgb & target = _targetValues[i];

			reddif   = target.red - prev.red;
			greendif = target.green - prev.green;
			bluedif  = target.blue - prev.blue;

			prev.red   += (reddif   < 0 ? -1:1) * ceil(k * abs(reddif));
			prev.green += (greendif < 0 ? -1:1) * ceil(k * abs(greendif));
			prev.blue  += (bluedif  < 0 ? -1:1) * ceil(k * abs(bluedif));
		}
		_previousTime = now;

		queueColors(_previousValues);
	}
}

void LinearColorSmoothing::queueColors(const std::vector<ColorRgb> & ledColors)
{
	if (_outputDelay == 0)
	{
		// No output delay => immediate write
		_ledDevice->write(ledColors);
	}
	else
	{
		// Push new colors in the delay-buffer
		_outputQueue.push_back(ledColors);

		// If the delay-buffer is filled pop the front and write to device
		if (_outputQueue.size() > 0 )
		{
			if ( _outputQueue.size() > _outputDelay || !_writeToLedsEnable )
			{
				_ledDevice->write(_outputQueue.front());
				_outputQueue.pop_front();
			}
		}
	}
}
