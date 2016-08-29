# Configure minimum CMAKE version
cmake_minimum_required(VERSION 2.8)

# Set the project name
project(hyperion-aml)

# find QT
find_package(Qt5Widgets REQUIRED)

include_directories(
	${QT_INCLUDES}
)

set(Hyperion_AML_QT_HEADERS
	AmlogicWrapper.h
)

set(Hyperion_AML_HEADERS
)

set(Hyperion_AML_SOURCES
	AmlogicWrapper.cpp
)

QT5_WRAP_CPP(Hyperion_AML_HEADERS_MOC ${Hyperion_AML_QT_HEADERS})

add_library(${PROJECT_NAME}
	${Hyperion_AML_HEADERS}
	${Hyperion_AML_SOURCES}
	${Hyperion_AML_HEADERS_MOC}
)

target_link_libraries(${PROJECT_NAME}
	hyperion-utils
	amlogic-grabber
	pthread
)

qt5_use_modules(${PROJECT_NAME} Widgets Core Gui Network)
