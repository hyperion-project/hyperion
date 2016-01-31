#pragma once

// Utils includes
#include <utils/Image.h>

namespace hyperion
{
	///
	/// Result structure of the detected blackborder.
	///
	struct BlackBorder
	{
		/// Falg indicating if the border is unknown
		bool unknown;

		/// The size of the detected horizontal border
		int horizontalSize;

		/// The size of the detected vertical border
		int verticalSize;

		///
		/// Compares this BlackBorder to the given other BlackBorder
		///
		/// @param[in] other  The other BlackBorder
		///
		/// @return True if this is the same border as other
		///
		inline bool operator== (const BlackBorder& other) const
		{
			if (unknown)
			{
				return other.unknown;
			}

			return other.unknown==false && horizontalSize==other.horizontalSize && verticalSize==other.verticalSize;
		}
	};

	///
	/// The BlackBorderDetector performs detection of black-borders on a single image.
	/// The detector will search for the upper left corner of the picture in the frame.
	/// Based on detected black pixels it will give an estimate of the black-border.
	///
	class BlackBorderDetector
	{
	public:
		///
		/// Constructs a black-border detector
		/// @param[in] blackborderThreshold The threshold which the blackborder detector should use
		///
		BlackBorderDetector(double threshold);

		///
		/// Performs the actual black-border detection on the given image
		///
		/// @param[in] image  The image on which detection is performed
		///
		/// @return The detected (or not detected) black border info
		///

		uint8_t calculateThreshold(double blackborderThreshold);

		///
		/// default detection mode (3lines 4side detection)
		template <typename Pixel_T>
		BlackBorder process(const Image<Pixel_T> & image)
		{
			// test center and 33%, 66% of width/heigth
			// 33 and 66 will check left and top
			// center will check right and bottom sids
			int width = image.width();
			int height = image.height();
			int width33percent = width / 3;
			int height33percent = height / 3;
			int width66percent = width33percent * 2;
			int height66percent = height33percent * 2;
			int xCenter = width / 2;
			int yCenter = height / 2;


			int firstNonBlackXPixelIndex = -1;
			int firstNonBlackYPixelIndex = -1;

			// find first X pixel of the image
			for (int x = 0; x < width33percent; ++x)
			{
				const Pixel_T & color1 = image( (width - 1 - x), yCenter); // right side center line check
				const Pixel_T & color2 = image(x, height33percent);
				const Pixel_T & color3 = image(x, height66percent);
				if (!isBlack(color1) || !isBlack(color2) || !isBlack(color3))
				{
					firstNonBlackXPixelIndex = x;
					break;
				}
			}

			// find first Y pixel of the image
			for (int y = 0; y < height33percent; ++y)
			{
				const Pixel_T & color1 = image(xCenter, (height - 1 - y)); // bottom center line check
				const Pixel_T & color2 = image(width33percent, y );
				const Pixel_T & color3 = image(width66percent, y);
				if (!isBlack(color1) || !isBlack(color2) || !isBlack(color3))
				{
					firstNonBlackYPixelIndex = y;
					break;
				}
			}

			// Construct result
			BlackBorder detectedBorder;
			detectedBorder.unknown = firstNonBlackXPixelIndex == -1 || firstNonBlackYPixelIndex == -1;
			detectedBorder.horizontalSize = firstNonBlackYPixelIndex;
			detectedBorder.verticalSize = firstNonBlackXPixelIndex;
			return detectedBorder;
		}


		///
		/// classic detection mode (topleft single line mode)
		template <typename Pixel_T>
		BlackBorder process_classic(const Image<Pixel_T> & image)
		{
			// only test the topleft third of the image
			int width = image.width() /3;
			int height = image.height() / 3;
			int maxSize = std::max(width, height);

			int firstNonBlackXPixelIndex = -1;
			int firstNonBlackYPixelIndex = -1;

			// find some pixel of the image
			for (int i = 0; i < maxSize; ++i)
			{
				int x = std::min(i, width);
				int y = std::min(i, height);

				const Pixel_T & color = image(x, y);
				if (!isBlack(color))
				{
					firstNonBlackXPixelIndex = x;
					firstNonBlackYPixelIndex = y;
					break;
				}
			}

			// expand image to the left
			for(; firstNonBlackXPixelIndex > 0; --firstNonBlackXPixelIndex)
			{
				const Pixel_T & color = image(firstNonBlackXPixelIndex-1, firstNonBlackYPixelIndex);
				if (isBlack(color))
				{
					break;
				}
			}

			// expand image to the top
			for(; firstNonBlackYPixelIndex > 0; --firstNonBlackYPixelIndex)
			{
				const Pixel_T & color = image(firstNonBlackXPixelIndex, firstNonBlackYPixelIndex-1);
				if (isBlack(color))
				{
					break;
				}
			}

			// Construct result
			BlackBorder detectedBorder;
			detectedBorder.unknown = firstNonBlackXPixelIndex == -1 || firstNonBlackYPixelIndex == -1;
			detectedBorder.horizontalSize = firstNonBlackYPixelIndex;
			detectedBorder.verticalSize = firstNonBlackXPixelIndex;
			return detectedBorder;
		}


// osd detection mode (find x then y at detected x to avoid changes by osd overlays)
		template <typename Pixel_T>
		BlackBorder process_osd(const Image<Pixel_T> & image)
		{
			// find X position at height33 and height66 we check from the left side, Ycenter will check from right side
			// then we try to find a pixel at this X position from top and bottom and right side from top
			int width = image.width();
			int height = image.height();
			int width33percent = width / 3;
			int height33percent = height / 3;
//			int width66percent = width33percent * 2;
			int height66percent = height33percent * 2;
//			int xCenter = width / 2;
			int yCenter = height / 2;


			int firstNonBlackXPixelIndex = -1;
			int firstNonBlackYPixelIndex = -1;

			// find first X pixel of the image
			int x;
			for (x = 0; x < width33percent; ++x)
			{
				const Pixel_T & color1 = image( (width - 1 - x), yCenter); // right side center line check
				const Pixel_T & color2 = image(x, height33percent);
				const Pixel_T & color3 = image(x, height66percent);
				if (!isBlack(color1) || !isBlack(color2) || !isBlack(color3))
				{
					firstNonBlackXPixelIndex = x;
					break;
				}
			}

			// find first Y pixel of the image
			for (int y = 0; y < height33percent; ++y)
			{
				const Pixel_T & color1 = image(x, (height - 1 - y)); // left side bottom check
				const Pixel_T & color2 = image(x, y );// left side top check
				const Pixel_T & color3 = image( (width - 1 - x), y); // right side top check
				if (!isBlack(color1) || !isBlack(color2) || !isBlack(color3))
				{
					firstNonBlackYPixelIndex = y;
					break;
				}
			}

			// Construct result
			BlackBorder detectedBorder;
			detectedBorder.unknown = firstNonBlackXPixelIndex == -1 || firstNonBlackYPixelIndex == -1;
			detectedBorder.horizontalSize = firstNonBlackYPixelIndex;
			detectedBorder.verticalSize = firstNonBlackXPixelIndex;
			return detectedBorder;
		}



	private:

		///
		/// Checks if a given color is considered black and therefor could be part of the border.
		///
		/// @param[in] color  The color to check
		///
		/// @return True if the color is considered black else false
		///
		template <typename Pixel_T>
		inline bool isBlack(const Pixel_T & color)
		{
			// Return the simple compare of the color against black
			return color.red < _blackborderThreshold && color.green < _blackborderThreshold && color.blue < _blackborderThreshold;
		}

	private:
		/// Threshold for the blackborder detector [0 .. 255]
		const uint8_t _blackborderThreshold;

	};
} // end namespace hyperion
