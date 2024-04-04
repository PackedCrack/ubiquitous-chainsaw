if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_custom_target(CPPCHECK
		COMMAND "C:/Program Files/Git/bin/bash.exe" "${PROJECT_SOURCE_DIR}/cppcheck.sh"
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