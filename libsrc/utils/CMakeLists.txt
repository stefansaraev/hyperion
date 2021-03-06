# Define the current source locations
SET(CURRENT_HEADER_DIR ${CMAKE_SOURCE_DIR}/include/utils)
SET(CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libsrc/utils)

add_library(hyperion-utils
	${CURRENT_HEADER_DIR}/ColorBgr.h
	${CURRENT_SOURCE_DIR}/ColorBgr.cpp
	${CURRENT_HEADER_DIR}/ColorRgb.h
	${CURRENT_SOURCE_DIR}/ColorRgb.cpp
	${CURRENT_HEADER_DIR}/Image.h
	${CURRENT_HEADER_DIR}/Sleep.h

	${CURRENT_HEADER_DIR}/Logger.h
	${CURRENT_SOURCE_DIR}/Logger.cpp

	${CURRENT_HEADER_DIR}/PixelFormat.h
	${CURRENT_HEADER_DIR}/VideoMode.h

	${CURRENT_HEADER_DIR}/ImageResampler.h
	${CURRENT_SOURCE_DIR}/ImageResampler.cpp

	${CURRENT_HEADER_DIR}/jsonschema/JsonFactory.h
	${CURRENT_HEADER_DIR}/jsonschema/JsonSchemaChecker.h
	${CURRENT_SOURCE_DIR}/jsonschema/JsonSchemaChecker.cpp
)

qt5_use_modules(hyperion-utils Widgets)

target_link_libraries(hyperion-utils
	jsoncpp
	${QT_LIBRARIES})
