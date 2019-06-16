#pragma once

// Qt includes
#include <QObject>

// util includes
#include <utils/Image.h>
#include <utils/ColorRgb.h>

// grabber includes
#include <opencv2/opencv.hpp>

/// Capture class for OpenCV
class OpenCVGrabber : public QObject
{
    Q_OBJECT

public:
    OpenCVGrabber(int input, int width, int height);
    virtual ~OpenCVGrabber();

public slots:
    void grabFrame(Image<ColorRgb> & image);

private:
    cv::VideoCapture _capture;
};
