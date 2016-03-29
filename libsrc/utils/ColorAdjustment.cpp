#include <iostream>
#include <utils/ColorAdjustment.h>

ColorAdjustment::ColorAdjustment() :
	_redChannel.valueRed(255),
	_redChannel.valueGreen(0),
	_redChannel.valueBlue(0),
	_greenChannel.valueRed(0),
	_greenChannel.valueGreen(255),
	_greenChannel.valueBlue(0),
	_blueChannel.valueRed(0),
	_blueChannel.valueGreen(0),
	_blueChannel.valueBlue(255)
{
}

ColorAdjustment::ColorAdjustment(ColorAdjustmentValues *redChannel, ColorAdjustmentValues *greenChannel, ColorAdjustmentValues *blueChannel) :
	_redChannel.valueRed(redChannel->valueRed);
	_redChannel.valueGreen(redChannel->valueGreen),
	_redChannel.valueBlue(redChannel->valueBlue),
	_greenChannel.valueRed(greenChannel->valueRed),
	_greenChannel.valueGreen(greenChannel->valueGreen),
	_greenChannel.valueBlue(greenChannel->valueBlue),
	_blueChannel.valueRed(blueChannel->valueRed),
	_blueChannel.valueGreen(blueChannel->valueGreen),
	_blueChannel.valueBlue(blueChannel->valueBlue)
{
}

ColorAdjustment::~ColorAdjustment()
{
}

void ColorAdjustment::setRedChannelAdjustment(ColorAdjustmentValues *Channel);
{
}

void ColorAdjustment::setGreenChannelAdjustment(ColorAdjustmentValues *Channel);
{
}

void ColorAdjustment::setBlueChannelAdjustment(ColorAdjustmentValues *Channel);
{
}


void ColorAdjustment::adjust(uint8_t & red, uint8_t & green, uint8_t & blue) const
{
  float rr = red * _redChannel.valueRed / 255.0f;
  float rg = red * _redChannel.valueGreen / 255.0f;
  float rb = red * _redChannel.valueBlue / 255.0f;
  float gr = green * _greenChannel.valueRed / 255.0f;
  float gg = green * _greenChannel.valueGreen / 255.0f;
  float gb = green * _greenChannel.valueBlue / 255.0f;
  float br = blue * _blueChannel.valueRed / 255.0f;
  float bg = blue * _blueChannel.valueGreen / 255.0f;
  float bb = blue * _blueChannel.valueBlue / 255.0f;
  
  float r = rr + gr + br;
  float g = rg + gg + bg;
  float b = rb + gb + bb;
  
  if ( r > (float)_redChannel.valueRed )
    red = _redChannel.valueRed;
  else
    red = (uint8_t)(r);
  
  if ( g > (float)_greenChannel.valueGreen )
    green = _greenChannel.valueGreen;
  else
    green = (uint8_t)(g);
  
  if ( b > (float)_blueChannel.valueBlue )
    blue = _blueChannel.valueBlue;
  else
    blue = (uint8_t)(b);

}

