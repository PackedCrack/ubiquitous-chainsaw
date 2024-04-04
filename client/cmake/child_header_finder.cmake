add_custom_target(CHILD_HEADER_FINDER
    COMMAND python3 child_header_finder.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Running Windows.h child header finder script."
    VERBATIM
)