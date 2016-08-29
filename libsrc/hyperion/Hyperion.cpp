
// STL includes
#include <cassert>

// QT includes
#include <QDateTime>
#include <QThread>
#include <QRegExp>
#include <QString>
#include <QStringList>

// JsonSchema include
#include <utils/jsonschema/JsonFactory.h>

// hyperion include
#include <hyperion/Hyperion.h>
#include <hyperion/ImageProcessorFactory.h>

// Leddevice includes
#include <leddevice/LedDevice.h>
#include <leddevice/LedDeviceFactory.h>

#include "LinearColorSmoothing.h"

ColorOrder Hyperion::createColorOrder(const Json::Value &deviceConfig)
{
	// deprecated: force BGR when the deprecated flag is present and set to true
	if (deviceConfig.get("bgr-output", false).asBool())
	{
		return ORDER_BGR;
	}

	std::string order = deviceConfig.get("colorOrder", "rgb").asString();
	if (order == "rgb")
	{
		return ORDER_RGB;
	}
	else if (order == "bgr")
	{
		return ORDER_BGR;
	}
	else if (order == "rbg")
	{
		return ORDER_RBG;
	}
	else if (order == "brg")
	{
		return ORDER_BRG;
	}
	else if (order == "gbr")
	{
		return ORDER_GBR;
	}
	else if (order == "grb")
	{
		return ORDER_GRB;
	}
	else
	{
		std::cout << "HYPERION ERROR: Unknown color order defined (" << order << "). Using RGB." << std::endl;
	}

	return ORDER_RGB;
}

LedString Hyperion::createLedString(const Json::Value& ledsConfig, const ColorOrder deviceOrder)
{
	LedString ledString;

	const std::string deviceOrderStr = colorOrderToString(deviceOrder);
	for (const Json::Value& ledConfig : ledsConfig)
	{
		Led led;
		led.index = ledConfig["index"].asInt();

		const Json::Value& hscanConfig = ledConfig["hscan"];
		const Json::Value& vscanConfig = ledConfig["vscan"];
		led.minX_frac = std::max(0.0, std::min(1.0, hscanConfig["minimum"].asDouble()));
		led.maxX_frac = std::max(0.0, std::min(1.0, hscanConfig["maximum"].asDouble()));
		led.minY_frac = std::max(0.0, std::min(1.0, vscanConfig["minimum"].asDouble()));
		led.maxY_frac = std::max(0.0, std::min(1.0, vscanConfig["maximum"].asDouble()));

		// Fix if the user swapped min and max
		if (led.minX_frac > led.maxX_frac)
		{
			std::swap(led.minX_frac, led.maxX_frac);
		}
		if (led.minY_frac > led.maxY_frac)
		{
			std::swap(led.minY_frac, led.maxY_frac);
		}

		// Get the order of the rgb channels for this led (default is device order)
		const std::string ledOrderStr = ledConfig.get("colorOrder", deviceOrderStr).asString();
		led.colorOrder = stringToColorOrder(ledOrderStr);

		ledString.leds().push_back(led);
	}

	// Make sure the leds are sorted (on their indices)
	std::sort(ledString.leds().begin(), ledString.leds().end(), [](const Led& lhs, const Led& rhs){ return lhs.index < rhs.index; });

	return ledString;
}

LedDevice * Hyperion::createColorSmoothing(const Json::Value & smoothingConfig, LedDevice * ledDevice)
{
	std::string type = smoothingConfig.get("type", "none").asString();
	std::transform(type.begin(), type.end(), type.begin(), ::tolower);

	if (type == "none")
	{
		std::cout << "HYPERION INFO: Not creating any smoothing" << std::endl;
		return ledDevice;
	}
	else if (type == "linear")
	{
		if (!smoothingConfig.isMember("time_ms"))
		{
			std::cout << "HYPERION ERROR: Unable to create smoothing of type linear because of missing parameter 'time_ms'" << std::endl;
		}
		else if (!smoothingConfig.isMember("updateFrequency"))
		{
			std::cout << "HYPERION ERROR: Unable to create smoothing of type linear because of missing parameter 'updateFrequency'" << std::endl;
		}
		else
		{
			std::cout << "INFO: Creating linear smoothing" << std::endl;
			return new LinearColorSmoothing(
					ledDevice,
		            smoothingConfig.get("updateFrequency", 25.0).asDouble(),
		            smoothingConfig.get("time_ms", 200).asInt(),
		            smoothingConfig.get("updateDelay", 0).asUInt(),
		            smoothingConfig.get("continuousOutput", true).asBool()
		            );
		}
	}
	else
	{
		std::cout << "HYPERION ERROR: Unable to create smoothing of type " << type << std::endl;
	}

	return ledDevice;
}

Hyperion::Hyperion(const Json::Value &jsonConfig, const std::string configFile) :
	_ledString(createLedString(jsonConfig["leds"], createColorOrder(jsonConfig["device"]))),
	_muxer(_ledString.leds().size()),
	_device(LedDeviceFactory::construct(jsonConfig["device"])),
	_jsonConfig(jsonConfig),
	_configFile(configFile),
	_timer()
{
	// initialize the image processor factory
	ImageProcessorFactory::getInstance().init(
				_ledString
	);

	// initialize the color smoothing filter
	_device = createColorSmoothing(jsonConfig["color"]["smoothing"], _device);

	// setup the timer
	_timer.setSingleShot(true);
	QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));

	// initialize the leds
	update();
}


Hyperion::~Hyperion()
{
	// switch off all leds
	clearall();
	_device->switchOff();

	// Delete the Led-String
	delete _device;
}

unsigned Hyperion::getLedCount() const
{
	return _ledString.leds().size();
}

void Hyperion::setColor(int priority, const ColorRgb &color, const int timeout_ms)
{
	// create led output
	std::vector<ColorRgb> ledColors(_ledString.leds().size(), color);

	// set colors
	setColors(priority, ledColors, timeout_ms);
}

void Hyperion::setColors(int priority, const std::vector<ColorRgb>& ledColors, const int timeout_ms)
{
	if (timeout_ms > 0)
	{
		const uint64_t timeoutTime = QDateTime::currentMSecsSinceEpoch() + timeout_ms;
		_muxer.setInput(priority, ledColors, timeoutTime);
	}
	else
	{
		_muxer.setInput(priority, ledColors);
	}

	if (priority == _muxer.getCurrentPriority())
	{
		update();
	}
}

void Hyperion::clear(int priority)
{
	if (_muxer.hasPriority(priority))
	{
		_muxer.clearInput(priority);

		// update leds if necessary
		if (priority < _muxer.getCurrentPriority())
		{
			update();
		}
	}
}

void Hyperion::clearall()
{
	_muxer.clearAll();

	// update leds
	update();
}

int Hyperion::getCurrentPriority() const
{
	return _muxer.getCurrentPriority();
}

QList<int> Hyperion::getActivePriorities() const
{
	return _muxer.getPriorities();
}

const Hyperion::InputInfo &Hyperion::getPriorityInfo(const int priority) const
{
	return _muxer.getInputInfo(priority);
}

void Hyperion::update()
{
	// Update the muxer, cleaning obsolete priorities
	_muxer.setCurrentTime(QDateTime::currentMSecsSinceEpoch());

	// Obtain the current priority channel
	int priority = _muxer.getCurrentPriority();
	const PriorityMuxer::InputInfo & priorityInfo  = _muxer.getInputInfo(priority);

	std::vector<ColorRgb> ledColors = priorityInfo.ledColors;
	const std::vector<Led>& leds = _ledString.leds();
	int i = 0;
	for (ColorRgb& color : ledColors)
	{
		const ColorOrder ledColorOrder = leds.at(i).colorOrder;
		// correct the color byte order
		switch (ledColorOrder)
		{
		case ORDER_RGB:
			// leave as it is
			break;
		case ORDER_BGR:
			std::swap(color.red, color.blue);
			break;
		case ORDER_RBG:
			std::swap(color.green, color.blue);
			break;
		case ORDER_GRB:
			std::swap(color.red, color.green);
			break;
		case ORDER_GBR:
		{
			std::swap(color.red, color.green);
			std::swap(color.green, color.blue);
			break;
		}
		case ORDER_BRG:
		{
			std::swap(color.red, color.blue);
			std::swap(color.green, color.blue);
			break;
		}
		}
		i++;
	}

	// Write the data to the device
	_device->write(ledColors);

	// Start the timeout-timer
	if (priorityInfo.timeoutTime_ms == -1)
	{
		_timer.stop();
	}
	else
	{
		int timeout_ms = std::max(0, int(priorityInfo.timeoutTime_ms - QDateTime::currentMSecsSinceEpoch()));
		_timer.start(timeout_ms);
	}
}
