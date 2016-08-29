#pragma once

// stl includes
#include <list>

// QT includes
#include <QObject>
#include <QTimer>

// hyperion-utils includes
#include <utils/Image.h>

// Hyperion includes
#include <hyperion/LedString.h>
#include <hyperion/PriorityMuxer.h>

// Forward class declaration
class LedDevice;
///
/// The main class of Hyperion. This gives other 'users' access to the attached LedDevice through
/// the priority muxer.
///
class Hyperion : public QObject
{
	Q_OBJECT
public:
	///  Type definition of the info structure used by the priority muxer
	typedef PriorityMuxer::InputInfo InputInfo;

	///
	/// RGB-Color channel enumeration
	///
	enum RgbChannel
	{
		RED, GREEN, BLUE, INVALID
	};

	///
	/// Enumeration of the possible color (color-channel) transforms
	///
	enum Transform
	{
		SATURATION_GAIN, VALUE_GAIN, THRESHOLD, GAMMA, BLACKLEVEL, WHITELEVEL
	};

	///
	/// Constructs the Hyperion instance based on the given Json configuration
	///
	/// @param[in] jsonConfig The Json configuration
	///
	Hyperion(const Json::Value& jsonConfig, const std::string configFile);

	///
	/// Destructor; cleans up resourcess
	///
	~Hyperion();

	///
	/// Returns the number of attached leds
	///
	unsigned getLedCount() const;
	
	///
	/// Returns the current priority
	///
	/// @return The current priority
	///
	int getCurrentPriority() const;
	///
	/// Returns a list of active priorities
	///
	/// @return The list with priorities
	///
	QList<int> getActivePriorities() const;

	///
	/// Returns the information of a specific priorrity channel
	///
	/// @param[in] priority  The priority channel
	///
	/// @return The information of the given
	///
	/// @throw std::runtime_error when the priority channel does not exist
	///
	const InputInfo& getPriorityInfo(const int priority) const;

	/// 
	const Json::Value& getJsonConfig() { return _jsonConfig; };
	
	std::string getConfigFileName() { return _configFile; };

public slots:
	///
	/// Writes a single color to all the leds for the given time and priority
	///
	/// @param[in] priority The priority of the written color
	/// @param[in] ledColor The color to write to the leds
	/// @param[in] timeout_ms The time the leds are set to the given color [ms]
	///
	void setColor(int priority, const ColorRgb &ledColor, const int timeout_ms);

	///
	/// Writes the given colors to all leds for the given time and priority
	///
	/// @param[in] priority The priority of the written colors
	/// @param[in] ledColors The colors to write to the leds
	/// @param[in] timeout_ms The time the leds are set to the given colors [ms]
	///
	void setColors(int priority, const std::vector<ColorRgb> &ledColors, const int timeout_ms);

	///
	/// Clears the given priority channel. This will switch the led-colors to the colors of the next
	/// lower priority channel (or off if no more channels are set)
	///
	/// @param[in] priority  The priority channel
	///
	void clear(int priority);

	///
	/// Clears all priority channels. This will switch the leds off until a new priority is written.
	///
	void clearall();

public:
	static ColorOrder createColorOrder(const Json::Value & deviceConfig);
	/**
	 * Construct the 'led-string' with the integration area definition per led and the color
	 * ordering of the RGB channels
	 * @param ledsConfig   The configuration of the led areas
	 * @param deviceOrder  The default RGB channel ordering
	 * @return The constructed ledstring
	 */
	static LedString createLedString(const Json::Value & ledsConfig, const ColorOrder deviceOrder);

	static LedDevice * createColorSmoothing(const Json::Value & smoothingConfig, LedDevice * ledDevice);
	
signals:
	/// Signal which is emitted when a priority channel is actively cleared
	/// This signal will not be emitted when a priority channel time out
	void channelCleared(int priority);

	/// Signal which is emitted when all priority channels are actively cleared
	/// This signal will not be emitted when a priority channel time out
	void allChannelsCleared();

private slots:
	///
	/// Updates the priority muxer with the current time and (re)writes the led color with applied
	/// transforms.
	///
	void update();

private:
	/// The specifiation of the led frame construction and picture integration
	LedString _ledString;

	/// The priority muxer
	PriorityMuxer _muxer;

	/// The actual LedDevice
	LedDevice * _device;

	// json configuration
	const Json::Value& _jsonConfig;

	// the name of config file
	std::string _configFile;

	/// The timer for handling priority channel timeouts
	QTimer _timer;
};
