// C++ includes
#include <cassert>
#include <csignal>
#include <vector>
#include <unistd.h>

// QT includes
#include <QCoreApplication>
#include <QResource>
#include <QLocale>
#include <QFile>

// config includes
#include "HyperionConfig.h"

// Json-Schema includes
#include <utils/jsonschema/JsonFactory.h>

// Hyperion includes
#include <hyperion/Hyperion.h>

#ifdef ENABLE_FB
// Framebuffer grabber includes
#include <grabber/FramebufferWrapper.h>
#endif

#ifdef ENABLE_AMLOGIC
#include <grabber/AmlogicWrapper.h>
#endif

// XBMC Video checker includes
#include <xbmcvideochecker/XBMCVideoChecker.h>

#include <sys/prctl.h> 
#include <utils/Logger.h>

void signal_handler(const int signum)
{
	QCoreApplication::quit();

	// reset signal handler to default (in case this handler is not capable of stopping)
	signal(signum, SIG_DFL);
}


Json::Value loadConfig(const std::string & configFile)
{
	// make sure the resources are loaded (they may be left out after static linking)
	Q_INIT_RESOURCE(resource);

	// read the json schema from the resource
	QResource schemaData(":/hyperion-schema");
	assert(schemaData.isValid());

	Json::Reader jsonReader;
	Json::Value schemaJson;
	if (!jsonReader.parse(reinterpret_cast<const char *>(schemaData.data()), reinterpret_cast<const char *>(schemaData.data()) + schemaData.size(), schemaJson, false))
	{
		throw std::runtime_error("ERROR: Json schema wrong: " + jsonReader.getFormattedErrorMessages())	;
	}
	JsonSchemaChecker schemaChecker;
	schemaChecker.setSchema(schemaJson);

	const Json::Value jsonConfig = JsonFactory::readJson(configFile);
	schemaChecker.validate(jsonConfig);

	return jsonConfig;
}


void startNewHyperion(int parentPid, std::string hyperionFile, std::string configFile)
{
	if ( fork() == 0 )
	{
		sleep(3);
		execl(hyperionFile.c_str(), hyperionFile.c_str(), "--parent", QString::number(parentPid).toStdString().c_str(), configFile.c_str(), NULL);
		exit(0);
	}
}


// create XBMC video checker if the configuration is present
void startXBMCVideoChecker(const Json::Value &config, XBMCVideoChecker* &xbmcVideoChecker)
{
	if (config.isMember("xbmcVideoChecker"))
	{
		const Json::Value & videoCheckerConfig = config["xbmcVideoChecker"];
		xbmcVideoChecker = new XBMCVideoChecker(
			videoCheckerConfig["xbmcAddress"].asString(),
			videoCheckerConfig["xbmcTcpPort"].asUInt(),
			videoCheckerConfig["grabVideo"].asBool(),
			videoCheckerConfig["grabPictures"].asBool(),
			videoCheckerConfig["grabAudio"].asBool(),
			videoCheckerConfig["grabMenu"].asBool(),
			videoCheckerConfig.get("grabPause", true).asBool(),
			videoCheckerConfig.get("grabScreensaver", true).asBool(),
			videoCheckerConfig.get("enable3DDetection", true).asBool());

		xbmcVideoChecker->start();
		std::cout << "INFO: Kodi checker created and started" << std::endl;
	}
}

#ifdef ENABLE_AMLOGIC
void startGrabberAmlogic(const Json::Value &config, Hyperion &hyperion, XBMCVideoChecker* &xbmcVideoChecker, AmlogicWrapper* &amlGrabber)
{
	// Construct and start the framebuffer grabber if the configuration is present
	if (config.isMember("amlgrabber"))
	{
		const Json::Value & grabberConfig = config["amlgrabber"];
		amlGrabber = new AmlogicWrapper(
			grabberConfig["width"].asUInt(),
			grabberConfig["height"].asUInt(),
			grabberConfig["frequency_Hz"].asUInt(),
			grabberConfig.get("priority",900).asInt(),
			&hyperion);

		if (xbmcVideoChecker != nullptr)
		{
			QObject::connect(xbmcVideoChecker, SIGNAL(grabbingMode(GrabbingMode)), amlGrabber, SLOT(setGrabbingMode(GrabbingMode)));
			QObject::connect(xbmcVideoChecker, SIGNAL(videoMode(VideoMode)),       amlGrabber, SLOT(setVideoMode(VideoMode)));
		}

		amlGrabber->start();
		std::cout << "INFO: AMLOGIC grabber created and started" << std::endl;
	}
}
#endif


#ifdef ENABLE_FB
void startGrabberFramebuffer(const Json::Value &config, Hyperion &hyperion, XBMCVideoChecker* &xbmcVideoChecker, FramebufferWrapper* &fbGrabber)
{
	// Construct and start the framebuffer grabber if the configuration is present
	if (config.isMember("framebuffergrabber") || config.isMember("framegrabber"))
	{
		const Json::Value & grabberConfig = config.isMember("framebuffergrabber")? config["framebuffergrabber"] : config["framegrabber"];
		fbGrabber = new FramebufferWrapper(
			grabberConfig.get("device", "/dev/fb0").asString(),
			grabberConfig["width"].asUInt(),
			grabberConfig["height"].asUInt(),
			grabberConfig["frequency_Hz"].asUInt(),
			grabberConfig.get("priority",900).asInt(),
			&hyperion);

		if (xbmcVideoChecker != nullptr)
		{
			QObject::connect(xbmcVideoChecker, SIGNAL(grabbingMode(GrabbingMode)), fbGrabber, SLOT(setGrabbingMode(GrabbingMode)));
			QObject::connect(xbmcVideoChecker, SIGNAL(videoMode(VideoMode)), fbGrabber, SLOT(setVideoMode(VideoMode)));
		}

		fbGrabber->start();
		std::cout << "INFO: Framebuffer grabber created and started" << std::endl;
	}
}
#endif


int main(int argc, char** argv)
{
	std::cout
		<< "Hyperion Ambilight Deamon (" << getpid() << ")" << std::endl
		<< "\tVersion   : " << HYPERION_VERSION_ID << std::endl
		<< "\tBuild Time: " << __DATE__ << " " << __TIME__ << std::endl;

	// Initialising QCoreApplication
	QCoreApplication app(argc, argv);

	signal(SIGINT,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGCHLD, signal_handler);

	// force the locale
	setlocale(LC_ALL, "C");
	QLocale::setDefault(QLocale::c());

	std::vector<std::string> configFiles;
	for(int i = 1; i < argc; i++)
		configFiles.push_back(argv[i]);

	if (configFiles.size() == 0)
	{
		std::cout << "ERROR: Missing required configuration file. Usage:" << std::endl;
		std::cout << "hyperiond <options ...> [config.file ...]" << std::endl;
		return 1;
	}

	int argvId = -1;
	for(size_t idx=0; idx < configFiles.size(); idx++) {
		if ( QFile::exists(configFiles[idx].c_str()))
		{
			if (argvId < 0) argvId=idx;
			else startNewHyperion(getpid(), argv[0], configFiles[idx].c_str());
		}
	}

	if ( argvId < 0)
	{
		std::cout << "ERROR: No valid config found " << std::endl;
		return 1;
	}

	const std::string configFile = configFiles[argvId];
	std::cout << "INFO: Selected configuration file: " << configFile.c_str() << std::endl;
	const Json::Value config = loadConfig(configFile);

	Hyperion hyperion(config, configFile);
	std::cout << "INFO: Hyperion started and initialised" << std::endl;

	XBMCVideoChecker * xbmcVideoChecker = nullptr;
	startXBMCVideoChecker(config, xbmcVideoChecker);

// ---- grabber -----

#ifdef ENABLE_AMLOGIC
	// Construct and start the framebuffer grabber if the configuration is present
	AmlogicWrapper * amlGrabber = nullptr;
	startGrabberAmlogic(config, hyperion, xbmcVideoChecker, amlGrabber);
#endif

#ifdef ENABLE_FB
	// Construct and start the framebuffer grabber if the configuration is present
	FramebufferWrapper * fbGrabber = nullptr;
	startGrabberFramebuffer(config, hyperion, xbmcVideoChecker, fbGrabber);
#endif

	// run the application
	int rc = app.exec();
	std::cout << "INFO: Application closed with code " << rc << std::endl;

	// Delete all component
#ifdef ENABLE_FB
	delete fbGrabber;
#endif
	delete xbmcVideoChecker;

	// leave application
	return rc;
}
