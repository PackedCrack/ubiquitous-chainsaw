if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_custom_target(CPPCHECK
		COMMAND cppcheck.sh
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMENT "Running CppCheck.."
		VERBATIM
	)
else()
	add_custom_target(CPPCHECK
		COMMAND /bin/bash cppcheck.sh
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMENT "Running CppCheck.."
		VERBATIM
	)
endif()