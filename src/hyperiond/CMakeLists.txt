
add_executable(hyperiond
		hyperiond.cpp)

target_link_libraries(hyperiond
		getoptPlusPlus
		hyperion
		xbmcvideochecker
)

if (ENABLE_FB)
	target_link_libraries(hyperiond framebuffer-grabber)
endif ()

if (ENABLE_AMLOGIC)
	target_link_libraries(hyperiond amlogic-grabber)
endif ()
