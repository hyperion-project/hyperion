#include "utils/ImageResampler.h"
#include "QImage"

ImageResampler::ImageResampler() :
	_horizontalDecimation(1),
	_verticalDecimation(1),
	_cropLeft(0),
	_cropRight(0),
	_cropTop(0),
	_cropBottom(0),
	_videoMode(VIDEO_2D)
{

}

ImageResampler::~ImageResampler()
{

}

void ImageResampler::setHorizontalPixelDecimation(int decimator)
{
	_horizontalDecimation = decimator;
}

void ImageResampler::setVerticalPixelDecimation(int decimator)
{
    _verticalDecimation = decimator;
}

void ImageResampler::setCropping(int cropLeft, int cropRight, int cropTop, int cropBottom)
{
    _cropLeft = cropLeft;
    _cropRight = cropRight;
    _cropTop = cropTop;
    _cropBottom = cropBottom;
}

void ImageResampler::set3D(VideoMode mode)
{
	_videoMode = mode;
}

void ImageResampler::processImage(const uint8_t * data, int width, int height, int lineLength, PixelFormat pixelFormat, Image<ColorRgb> &outputImage, bool dominantColor) const
{
    int cropLeft = _cropLeft;
    int cropRight = _cropRight;
    int cropTop = _cropTop;
    int cropBottom = _cropBottom;
    ColorRgb domColor;

    // handle 3D mode
    switch (_videoMode)
    {
    case VIDEO_3DSBS:
	cropRight = width/2;
	break;
    case VIDEO_3DTAB:
	cropBottom = height/2;
	break;
    default:
	break;
    }

    // calculate the output size
    int outputWidth = (width - cropLeft - cropRight - _horizontalDecimation/2 + _horizontalDecimation - 1) / _horizontalDecimation;
    int outputHeight = (height - cropTop - cropBottom - _verticalDecimation/2 + _verticalDecimation - 1) / _verticalDecimation;
    outputImage.resize(outputWidth, outputHeight);

    if (dominantColor)
    {
	QImage::Format format = QImage::Format_ARGB32_Premultiplied;
	QImage _qImage = QImage(reinterpret_cast<const uint8_t*>(data), width, height, lineLength, format).copy();
	_qImage = _qImage.scaledToHeight(100);
	
	uint8_t* data = _qImage.bits();
	quint64 size=_qImage.byteCount();
	
	quint64 map1[8][8][8]={0};
	
	for (quint64 i=0; i<size; i+=4)
	{
	    unsigned pixel = *(unsigned*)(data+i);
	    map1[(pixel>>(16+5))&7][(pixel>>(8+5))&7][(pixel>>5)&7]++;
	}
	
	quint64 bestr=0,bestg=0,bestb=0,max=0;
	
	for (int i=0;i<8;i++)
	    for (int j=0;j<8;j++)
		for (int k=0;k<8;k++)
		{
		    if (map1[i][j][k] > max)
		    {
			max=map1[i][j][k];
			bestr=i;
			bestg=j;
			bestb=k;
		    }
		}

	quint64 avgr=0, avgg=0, avgb=0, count=0;
	
	for (quint64 i=0; i<size; i+=4)
	{
	    unsigned pixel = *(unsigned*)(data+i);
	    int r = pixel>>16&0xFF, g=pixel>>8&0xFF, b=pixel&0xFF;
	    if ((r>>5&7)==bestr && (g>>5&7)==bestg && (b>>5&7)==bestb)
	    {
		avgr+=r;
		avgg+=g;
		avgb+=b;
		count++;
	    }
	}
	
	domColor.red = avgr/count;
	domColor.green = avgg/count;
	domColor.blue = avgb/count;
    }

    for (int yDest = 0, ySource = cropTop + _verticalDecimation/2; yDest < outputHeight; ySource += _verticalDecimation, ++yDest)
    {
	for (int xDest = 0, xSource = cropLeft + _horizontalDecimation/2; xDest < outputWidth; xSource += _horizontalDecimation, ++xDest)
	{
	    ColorRgb & rgb = outputImage(xDest, yDest);

	    if (dominantColor)
	    {
		rgb = domColor;
	    }
	    else
	    {
		switch (pixelFormat)
		{
		    case PIXELFORMAT_UYVY:
		    {
			int index = lineLength * ySource + xSource * 2;
			uint8_t y = data[index+1];
			uint8_t u = ((xSource&1) == 0) ? data[index  ] : data[index-2];
			uint8_t v = ((xSource&1) == 0) ? data[index+2] : data[index];
			yuv2rgb(y, u, v, rgb.red, rgb.green, rgb.blue);
		    }
			break;
		    case PIXELFORMAT_YUYV:
		    {
			int index = lineLength * ySource + xSource * 2;
			uint8_t y = data[index];
			uint8_t u = ((xSource&1) == 0) ? data[index+1] : data[index-1];
			uint8_t v = ((xSource&1) == 0) ? data[index+3] : data[index+1];
			yuv2rgb(y, u, v, rgb.red, rgb.green, rgb.blue);
		    }
			break;
		    case PIXELFORMAT_BGR16:
		    {
			int index = lineLength * ySource + xSource * 2;
			rgb.blue  = (data[index] & 0x1f) << 3;
			rgb.green = (((data[index+1] & 0x7) << 3) | (data[index] & 0xE0) >> 5) << 2;
			rgb.red   = (data[index+1] & 0xF8);
		    }
			break;
		    case PIXELFORMAT_BGR24:
		    {
			int index = lineLength * ySource + xSource * 3;
			rgb.blue  = data[index];
			rgb.green = data[index+1];
			rgb.red   = data[index+2];
		    }
			break;
		    case PIXELFORMAT_RGB32:
		    {
			int index = lineLength * ySource + xSource * 4;
			rgb.red   = data[index];
			rgb.green = data[index+1];
			rgb.blue  = data[index+2];
		    }
			break;
		    case PIXELFORMAT_BGR32:
		    {
			int index = lineLength * ySource + xSource * 4;
			rgb.blue  = data[index];
			rgb.green = data[index+1];
			rgb.red   = data[index+2];
		    }
			break;
		    case PIXELFORMAT_NO_CHANGE:
			std::cerr << "Invalid pixel format given" << std::endl;
			break;
		}
	    }
	}
    }
}

uint8_t ImageResampler::clamp(int x)
{
	return (x<0) ? 0 : ((x>255) ? 255 : uint8_t(x));
}

void ImageResampler::yuv2rgb(uint8_t y, uint8_t u, uint8_t v, uint8_t &r, uint8_t &g, uint8_t &b)
{
	// see: http://en.wikipedia.org/wiki/YUV#Y.27UV444_to_RGB888_conversion
	int c = y - 16;
	int d = u - 128;
	int e = v - 128;

	r = clamp((298 * c + 409 * e + 128) >> 8);
	g = clamp((298 * c - 100 * d - 208 * e + 128) >> 8);
	b = clamp((298 * c + 516 * d + 128) >> 8);
}
