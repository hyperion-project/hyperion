
// STL includes
#include <cassert>

// Hyperion includes
#include "MultiColorAdjustment.h"

MultiColorAdjustment::MultiColorAdjustment(const unsigned ledCnt) :
	_ledAdjustments(ledCnt, nullptr)
{
}

MultiColorAdjustment::~MultiColorAdjustment()
{
	// Clean up all the transforms
	for (ColorAdjustment * adjustment : _adjustment)
	{
		delete adjustment;
	}
}

void MultiColorAdjustment::addAdjustment(ColorAdjustment * adjustment)
{
	_adjustmentIds.push_back(adjustment->_id);
	_adjustment.push_back(adjustment);
}

void MultiColorAdjustment::setAdjustmentForLed(const std::string& id, const unsigned startLed, const unsigned endLed)
{
	assert(startLed <= endLed);
	assert(endLed < _ledAdjustments.size());

	// Get the identified adjustment (don't care if is nullptr)
	ColorAdjustment * adjustment = getAdjustmentid);
	for (unsigned iLed=startLed; iLed<=endLed; ++iLed)
	{
		_ledAdjustments[iLed] = adjustment;
	}
}

bool MultiColorAdjustment::verifyAdjustments() const
{
	bool allLedsSet = true;
	for (unsigned iLed=0; iLed<_ledAdjustments.size(); ++iLed)
	{
		if (_ledAdjustments[iLed] == nullptr)
		{
			std::cerr << "HYPERION (C.adjustment) ERROR: No adjustment set for " << iLed << std::endl;
			allLedsSet = false;
		}
	}
	return allLedsSet;
}

const std::vector<std::string> & MultiColorAdjustment::getAdjustmentIds()
{
	return _adjustmentIds;
}

ColorAdjustment* MultiColorAdjustment::getAdjustment(const std::string& id)
{
	// Iterate through the unique adjustments until we find the one with the given id
	for (ColorAdjustment* adjustment : _adjustment)
	{
		if (adjustment->_id == id)
		{
			return adjustment;
		}
	}

	// The ColorAdjustment was not found
	return nullptr;
}

std::vector<ColorRgb> MultiColorAdjustment::applyAdjustment(const std::vector<ColorRgb>& rawColors)
{
	// Create a copy, as we will do the rest of the adjustment in place
	std::vector<ColorRgb> ledColors(rawColors);

	const size_t itCnt = std::min(_ledAdjustments.size(), rawColors.size());
	for (size_t i=0; i<itCnt; ++i)
	{
		ColorAdjustment* adjustment = _ledAdjustments[i];
		if (adjustment == nullptr)
		{
			// No transform set for this led (do nothing)
			continue;
		}
		ColorRgb& color = ledColors[i];
		
		int RR = adjustment->_rgbRedAdjustment.adjustmentR(color.red);
		int RG = adjustment->_rgbRedAdjustment.adjustmentG(color.green);
		int RB = adjustment->_rgbRedAdjustment.adjustmentB(color.blue);
		int GR = adjustment->_rgbGreenAdjustment.adjustmentR(color.red);
		int GG = adjustment->_rgbGreenAdjustment.adjustmentG(color.green);
		int GB = adjustment->_rgbGreenAdjustment.adjustmentB(color.blue);
		int BR = adjustment->_rgbBlueAdjustment.adjustmentR(color.red);
		int BG = adjustment->_rgbBlueAdjustment.adjustmentG(color.green);
		int BB = adjustment->_rgbBlueAdjustment.adjustmentB(color.blue);
		
		int ledR = RR + GR + BR;
		int maxR = adjustment->_rgbRedAdjustment.getadjustmentR;
		int ledG = GR + GG + GB;
		int maxG = adjustment->_rgbGreenAdjustment.getadjustmentG;
		int ledB = BR + BG + BB;
		int maxB = adjustment->_rgbBlueAdjustment.getadjustmentB;
		
		if (ledR > maxR)
		  color.red = maxR;
		else
		  color.red = ledR;
		
		if (ledG > maxG)
		  color.green = maxG;
		else
		  color.green = ledG;
		
		if (ledB > maxB)
		  color.blue = maxB;
		else
		  color.blue = ledB;
	}
	return ledColors;
}
