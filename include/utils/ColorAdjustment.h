#pragma once

// STL includes
#include <cstdint>

/// Simple structure to contain the values of a color adjustment
struct ColorAdjustmentValues
{
	/// The value for the red color-channel
	int valueRed;
	/// The value for the green color-channel
	int valueGreen;
	/// The value for the blue color-channel
	int valueBlue;
};

///
/// Color adjustment of a RGB color value
///
class ColorAdjustment
{
public:
	///
	/// Default constructor
	///
	ColorAdjustment();

	///
	/// Constructor
	///
	/// @param saturationGain The used saturation gain
	/// @param valueGain The used value gain
	///
	ColorAdjustment(ColorAdjustmentValues *redChannel, ColorAdjustmentValues *greenChannel, ColorAdjustmentValues *blueChannel);

	///
	/// Destructor
	///
	~ColorAdjustment();

	///
	/// Updates the saturation gain
	///
	/// @param saturationGain New saturationGain
	///
	void setRedChannelAdjustment(ColorAdjustmentValues *Channel);
	
	void setGreenChannelAdjustment(ColorAdjustmentValues *Channel);
	
	void setBlueChannelAdjustment(ColorAdjustmentValues *Channel);

	///
	/// Returns the saturation gain
	///
	/// @return The current Saturation gain
	///
	ColorAdjustmentValues getRedChannelAdjustment() const;
	ColorAdjustmentValues getGreenChannelAdjustment() const;
	ColorAdjustmentValues getBlueChannelAdjustment() const;

	///
	/// Apply the transform the the given RGB values.
	///
	/// @param red The red color component
	/// @param green The green color component
	/// @param blue The blue color component
	///
	/// @note The values are updated in place.
	///
	void adjust(uint8_t & red, uint8_t & green, uint8_t & blue) const;


private:
	/// The saturation gain
	ColorAdjustmentValues _redChannel, _greenChannel, _blueChannel;
};

