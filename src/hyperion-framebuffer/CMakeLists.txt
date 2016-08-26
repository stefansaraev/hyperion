# Configure minimum CMAKE version
cmake_minimum_required(VERSION 2.8)

# Set the project name
project(hyperion-framebuffer)

# find QT
find_package(Qt5Widgets REQUIRED)

include_directories(
	${QT_INCLUDES}
)

set(Hyperion_FB_QT_HEADERS
	FramebufferWrapper.h
)

set(Hyperion_FB_HEADERS
)

set(Hyperion_FB_SOURCES
	hyperion-framebuffer.cpp
	FramebufferWrapper.cpp
)

QT5_WRAP_CPP(Hyperion_FB_HEADERS_MOC ${Hyperion_FB_QT_HEADERS})

add_executable( ${PROJECT_NAME}
	${Hyperion_FB_HEADERS}
	${Hyperion_FB_SOURCES}
	${Hyperion_FB_HEADERS_MOC}
)

target_link_libraries( ${PROJECT_NAME}
	getoptPlusPlus
	blackborder
	hyperion-utils
	framebuffer-grabber
	pthread
)

qt5_use_modules(${PROJECT_NAME} Widgets Core Gui Network)
