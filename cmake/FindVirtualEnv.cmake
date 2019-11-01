
find_program(VIRTUALENV_EXECUTABLE 
	NAMES virtualenv 
	HINTS $ENV{VIRTUALENV_DIR} 
	PATH_SUFFIXES bin 
	DOC "Sphinx documentation generator")

if (VIRUTALENV_EXECUTABLE) 
	execute_process(
        COMMAND $ { VIRTUALENV_EXECUTABLE } --version 
        RESULT_VARIABLE ret_code
        OUTPUT_VARIABLE version_string 
	ERROR_VARIABLE error_output
        OUTPUT_STRIP_TRAILING_WHITESPACE) 
        if (ret_code EQUAL 0 AND NOT ERROR_VARIABLE)
           set(VIRTUALENV_VERSION version_string) 
        endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(VirtualEnv 
	REQUIRED_VARS VIRTUALENV_EXECUTABLE 
	VERSION_VAR ${VIRTUALENV_VERSION})

mark_as_advanced(VIRTUALENV_EXECUTABLE)
