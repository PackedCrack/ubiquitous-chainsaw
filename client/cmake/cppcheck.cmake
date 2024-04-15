if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_custom_target(CPPCHECK_${MAIN_PROJECT}
		COMMAND PowerShell -ExecutionPolicy Bypass -File cppcheck.ps1
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMENT "Running CppCheck.."
		VERBATIM
	)
else()
	add_custom_target(CPPCHECK_${MAIN_PROJECT}
		COMMAND /bin/bash cppcheck.sh
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMENT "Running CppCheck.."
		VERBATIM
	)
endif()