// Stl includes
#include <string>
#include <algorithm>

// Build configuration
#include <HyperionConfig.h>

// Leddevice includes
#include <leddevice/LedDeviceFactory.h>

#include "LedDeviceAdalight.h"
#include "LedDeviceFile.h"

LedDevice * LedDeviceFactory::construct(const Json::Value & deviceConfig)
{
	std::cout << "LEDDEVICE INFO: configuration: " << deviceConfig << std::endl;

	std::string type = deviceConfig.get("type", "UNSPECIFIED").asString();
	std::transform(type.begin(), type.end(), type.begin(), ::tolower);

	LedDevice* device = nullptr;
	if (false) {}
	else if (type == "adalight")
	{
		const std::string output = deviceConfig["output"].asString();
		const unsigned rate      = deviceConfig["rate"].asInt();
		const int delay_ms       = deviceConfig["delayAfterConnect"].asInt();

		LedDeviceAdalight* deviceAdalight = new LedDeviceAdalight(output, rate, delay_ms);
		deviceAdalight->open();

		device = deviceAdalight;
	}
	else if (type == "file")
	{
		const std::string output = deviceConfig.get("output", "/dev/null").asString();
		device = new LedDeviceFile(output);
	}
	else
	{
		std::cout << "LEDDEVICE ERROR: Unknown/Unimplemented device " << type << std::endl;
		// Unknown / Unimplemented device
		exit(1);
	}
	return device;
}
