// Hyperion includes
#include <hyperion/ImageProcessor.h>
#include <hyperion/ImageToLedsMap.h>

using namespace hyperion;

//ImageProcessor::ImageProcessor(const LedString& ledString) :
ImageProcessor::ImageProcessor(const LedString& ledString) :
	_ledString(ledString),
	_imageToLeds(nullptr)
{
	// empty
}

ImageProcessor::~ImageProcessor()
{
	delete _imageToLeds;
}

unsigned ImageProcessor::getLedCount() const
{
	return _ledString.leds().size();
}

void ImageProcessor::setSize(const unsigned width, const unsigned height)
{
	// Check if the existing buffer-image is already the correct dimensions
	if (_imageToLeds && _imageToLeds->width() == width && _imageToLeds->height() == height)
	{
		return;
	}

	// Clean up the old buffer and mapping
	delete _imageToLeds;

	// Construct a new buffer and mapping
	_imageToLeds = new ImageToLedsMap(width, height, 0, 0, _ledString.leds());
}

bool ImageProcessor::getScanParameters(size_t led, double &hscanBegin, double &hscanEnd, double &vscanBegin, double &vscanEnd) const
{
	if (led < _ledString.leds().size())
	{
		const Led & l = _ledString.leds()[led];
		hscanBegin = l.minX_frac;
		hscanEnd = l.maxX_frac;
		vscanBegin = l.minY_frac;
		vscanEnd = l.maxY_frac;
	}

	return false;
}

