
#pragma once

// Utils includes
#include <utils/Image.h>

// Hyperion includes
#include <hyperion/ImageProcessorFactory.h>
#include <hyperion/LedString.h>
#include <hyperion/ImageToLedsMap.h>

///
/// The ImageProcessor translates an RGB-image to RGB-values for the leds. The processing is
/// performed in two steps. First the average color per led-region is computed. Second a
/// color-tranform is applied based on a gamma-correction.
///
class ImageProcessor
{
public:
	~ImageProcessor();

	///
	/// Returns the number of attached leds
	///
	unsigned getLedCount() const;

	///
	/// Specifies the width and height of 'incomming' images. This will resize the buffer-image to
	/// match the given size.
	/// NB All earlier obtained references will be invalid.
	///
	/// @param[in] width   The new width of the buffer-image
	/// @param[in] height  The new height of the buffer-image
	///
	void setSize(const unsigned width, const unsigned height);

	///
	/// Processes the image to a list of led colors. This will update the size of the buffer-image
	/// if required and call the image-to-leds mapping to determine the mean color per led.
	///
	/// @param[in] image  The image to translate to led values
	///
	/// @return The color value per led
	///
	template <typename Pixel_T>
	std::vector<ColorRgb> process(const Image<Pixel_T>& image)
	{
		// Ensure that the buffer-image is the proper size
		setSize(image.width(), image.height());

		// Create a result vector and call the 'in place' functionl
		std::vector<ColorRgb> colors = _imageToLeds->getMeanLedColor(image);

		// return the computed colors
		return colors;
	}

	///
	/// Determines the led colors of the image in the buffer.
	///
	/// @param[in] image  The image to translate to led values
	/// @param[out] ledColors  The color value per led
	///
	template <typename Pixel_T>
	void process(const Image<Pixel_T>& image, std::vector<ColorRgb>& ledColors)
	{
		// Ensure that the buffer-image is the proper size
		setSize(image.width(), image.height());

		// Determine the mean-colors of each led (using the existing mapping)
		_imageToLeds->getMeanLedColor(image, ledColors);
	}

	///
	/// Get the hscan and vscan parameters for a single led
	///
	/// @param[in] led Index of the led
	/// @param[out] hscanBegin begin of the hscan
	/// @param[out] hscanEnd end of the hscan
	/// @param[out] vscanBegin begin of the hscan
	/// @param[out] vscanEnd end of the hscan
	/// @return true if the parameters could be retrieved
	bool getScanParameters(size_t led, double & hscanBegin, double & hscanEnd, double & vscanBegin, double & vscanEnd) const;

private:
	/// Friend declaration of the factory for creating ImageProcessor's
	friend class ImageProcessorFactory;

	///
	/// Constructs an image-processor for translating an image to led-color values based on the
	/// given led-string specification
	///
	/// @param[in] ledString  The led-string specification
	///
	ImageProcessor(const LedString &ledString);

private:
	/// The Led-string specification
	const LedString _ledString;

	/// The mapping of image-pixels to leds
	hyperion::ImageToLedsMap* _imageToLeds;
};
