#pragma once

// Qt includes
#include <QTimer>

// Hyperion includes
#include <hyperion/Hyperion.h>
#include <hyperion/ImageProcessor.h>

// Grabber includes
#include <grabber/OpenCVGrabber.h>

class OpenCVWrapper : public QObject
{
    Q_OBJECT

public:
    OpenCVWrapper(int input,
                  int width,
                  int height,
                  int frequency,
                  int hyperionPriority,
                  Hyperion * hyperion);
    virtual ~OpenCVWrapper();

public slots:
    void start();

    void stop();

signals:
    void emitColors(int priority, const std::vector<ColorRgb> &ledColors, const int timeout_ms);
    void emitImage(int priority, const Image<ColorRgb> & image, const int timeout_ms);

private slots:
    void grabFrame();
    void checkSources();

private:
    /// The timeout of the led colors [ms]
    const int _timeout_ms;

    /// The priority of the led colors
    const int _priority;

    /// Grab frequency [Hz]
    const int _frequency;

    /// The OpenCV grabber
    OpenCVGrabber _grabber;

    /// The processor for transforming images to led colors
    ImageProcessor * _processor;

    /// The Hyperion instance
    Hyperion * _hyperion;

    /// The list with computed led colors
    std::vector<ColorRgb> _ledColors;

    /// Timer which tests if a higher priority source is active
    QTimer _priority_check_timer;

    QTimer _grab_timer;
};
