// QT includes
#include <QDebug>
#include <QDateTime>

// Hyperion includes
#include <hyperion/Hyperion.h>
#include <hyperion/ImageProcessorFactory.h>
#include <hyperion/ImageProcessor.h>

// Amlogic grabber includes
#include <grabber/AmlogicWrapper.h>
#include <grabber/AmlogicGrabber.h>


AmlogicWrapper::AmlogicWrapper(const unsigned grabWidth, const unsigned grabHeight, const unsigned updateRate_Hz, const int priority, Hyperion * hyperion) :
	_updateInterval_ms(1000/updateRate_Hz),
	_timeout_ms(2 * _updateInterval_ms),
	_priority(priority),
	_timer(),
	_image(grabWidth, grabHeight),
	_frameGrabber(new AmlogicGrabber(grabWidth, grabHeight)),
	_processor(ImageProcessorFactory::getInstance().newImageProcessor()),
	_ledColors(hyperion->getLedCount(), ColorRgb{0,0,0}),
	_hyperion(hyperion)
{
	// Configure the timer to generate events every n milliseconds
	_timer.setInterval(_updateInterval_ms);
	_timer.setSingleShot(false);

	_processor->setSize(grabWidth, grabHeight);

	// Connect the QTimer to this
	QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(action()));
}

AmlogicWrapper::~AmlogicWrapper()
{
	// Cleanup used resources (ImageProcessor and FrameGrabber)
	delete _processor;
	delete _frameGrabber;
}

void AmlogicWrapper::start()
{
	// Start the timer with the pre configured interval
	_timer.start();
}

void AmlogicWrapper::action()
{
	// Grab frame into the allocated image
	if (_frameGrabber->grabFrame(_image) < 0)
	{
		// Frame grab failed, maybe nothing playing or ....
		return;
	}

	_processor->process(_image, _ledColors);
	_hyperion->setColors(_priority, _ledColors, _timeout_ms);
}

void AmlogicWrapper::stop()
{
	// Stop the timer, effectivly stopping the process
	_timer.stop();
}

void AmlogicWrapper::setGrabbingMode(const GrabbingMode mode)
{
	switch (mode)
	{
	case GRABBINGMODE_VIDEO:
	case GRABBINGMODE_PAUSE:
		start();
		break;
	case GRABBINGMODE_AUDIO:
	case GRABBINGMODE_PHOTO:
	case GRABBINGMODE_MENU:
	case GRABBINGMODE_INVALID:
		start();
		break;
	case GRABBINGMODE_OFF:
		stop();
		break;
	}
}

void AmlogicWrapper::setVideoMode(const VideoMode mode)
{
	_frameGrabber->setVideoMode(mode);
}
