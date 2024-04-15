if(CMAKE_HOST_WIN32)
	add_custom_target(CPPCHECK_${COMPONENT_LIB}
		COMMAND PowerShell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/cppcheck.ps1
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Running CppCheck.."
		VERBATIM
	)
else()
	add_custom_target(CPPCHECK_${COMPONENT_LIB}
		COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cppcheck.sh
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMENT "Running CppCheck.."
		VERBATIM
	)
endif()