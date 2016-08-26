add_subdirectory(hyperiond)

if (ENABLE_AMLOGIC)
	add_subdirectory(hyperion-aml)
endif()

if(ENABLE_FB)
	add_subdirectory(hyperion-framebuffer)
endif()
