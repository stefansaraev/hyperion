# Define the current source locations
SET(CURRENT_HEADER_DIR ${CMAKE_SOURCE_DIR}/include/leddevice)
SET(CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libsrc/leddevice)

# Group the headers that go through the MOC compiler
SET(Leddevice_QT_HEADERS
	${CURRENT_SOURCE_DIR}/LedRs232Device.h
	${CURRENT_SOURCE_DIR}/LedDeviceAdalight.h
	${CURRENT_SOURCE_DIR}/LedDeviceFile.h
)

SET(Leddevice_HEADERS
	${CURRENT_HEADER_DIR}/LedDevice.h
	${CURRENT_HEADER_DIR}/LedDeviceFactory.h

	${CURRENT_SOURCE_DIR}/LedDeviceFile.h
)

SET(Leddevice_SOURCES
	${CURRENT_SOURCE_DIR}/LedDeviceFactory.cpp

	${CURRENT_SOURCE_DIR}/LedRs232Device.cpp

	${CURRENT_SOURCE_DIR}/LedDeviceAdalight.cpp
	${CURRENT_SOURCE_DIR}/LedDeviceFile.cpp
)

QT5_WRAP_CPP(Leddevice_HEADERS_MOC ${Leddevice_QT_HEADERS})

add_library(leddevice
	${Leddevice_HEADERS}
	${Leddevice_QT_HEADERS}
	${Leddevice_HEADERS_MOC}
	${Leddevice_SOURCES}
)

qt5_use_modules(leddevice Widgets Network)

target_link_libraries(leddevice
	hyperion-utils
	serialport
	${QT_LIBRARIES}
)
