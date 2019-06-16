#include <iostream>
#include <sstream>
#include <stdexcept>
#include <opencv2/opencv.hpp>

#include "grabber/OpenCVGrabber.h"


OpenCVGrabber::OpenCVGrabber(int input, int width, int height)
    : _capture(input)
{
    if (width && height) {
        _capture.set(CV_CAP_PROP_FRAME_WIDTH, width);
        _capture.set(CV_CAP_PROP_FRAME_HEIGHT, height);
    }
}

OpenCVGrabber::~OpenCVGrabber()
{
}

void OpenCVGrabber::grabFrame(Image<ColorRgb> & image)
{
    cv::Mat frame;
    _capture >> frame;

    const int width = frame.cols, height = frame.rows;

    cv::Mat rgbFrame(width, height, CV_8UC3);
    cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);

    image.resize(width, height);
    memcpy(image.memptr(), rgbFrame.ptr(), width * height * 3);
}
