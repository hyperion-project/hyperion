#include <QMetaType>

#include <grabber/OpenCVWrapper.h>

#include <hyperion/ImageProcessorFactory.h>

OpenCVWrapper::OpenCVWrapper(
        int input,
        int width,
        int height,
        int frequency,
        int hyperionPriority,
        Hyperion *hyperion) :
    _timeout_ms(1000),
    _priority(hyperionPriority),
    _frequency(frequency),
    _grabber(input, width, height),
    _processor(ImageProcessorFactory::getInstance().newImageProcessor()),
    _hyperion(hyperion),
    _ledColors(hyperion->getLedCount(), ColorRgb{0,0,0}),
    _priority_check_timer(),
    _grab_timer()
{
    // register the image type
    qRegisterMetaType<Image<ColorRgb>>("Image<ColorRgb>");
    qRegisterMetaType<std::vector<ColorRgb>>("std::vector<ColorRgb>");

    // send color data to Hyperion using a queued connection to handle the data over to the main event loop
    QObject::connect(
                this, SIGNAL(emitColors(int,std::vector<ColorRgb>,int)),
                _hyperion, SLOT(setColors(int,std::vector<ColorRgb>,int)),
                Qt::QueuedConnection);

    // setup the higher prio source checker
    // this will disable the grabber when a source with hisher priority is active
    _priority_check_timer.setInterval(500);
    _priority_check_timer.setSingleShot(false);
    QObject::connect(&_priority_check_timer, SIGNAL(timeout()), this, SLOT(checkSources()));
    _priority_check_timer.start();

    _grab_timer.setInterval(1000 / _frequency);
    _grab_timer.setSingleShot(false);

    QObject::connect(&_grab_timer, SIGNAL(timeout()), this, SLOT(grabFrame()));
}

OpenCVWrapper::~OpenCVWrapper()
{
    delete _processor;
}

void OpenCVWrapper::start()
{
    if (_grab_timer.isActive())
        return;

    _grab_timer.start();
    std::cout << "OPENCVGRABBER INFO: started" << std::endl;
}

void OpenCVWrapper::stop()
{
    if (!_grab_timer.isActive())
        return;

    _grab_timer.stop();
    std::cout << "OPENCVGRABBER INFO: stopped" << std::endl;
}

void OpenCVWrapper::grabFrame()
{
    Image<ColorRgb> image;

    _grabber.grabFrame(image);

    // process the new image
    _processor->process(image, _ledColors);

    // forward to other hyperions
    emit emitImage(_priority, image, _timeout_ms);

    // send colors to Hyperion
    emit emitColors(_priority, _ledColors, _timeout_ms);
}

void OpenCVWrapper::checkSources()
{
    QList<int> activePriorities = _hyperion->getActivePriorities();

    for (int x : activePriorities)
    {
        if (x < _priority)
        {
            // found a higher priority source: grabber should be disabled
            stop();
            return;
        }
    }

    // no higher priority source was found: grabber should be enabled
    start();
}
