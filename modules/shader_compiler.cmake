# The MIT License (MIT)
#
# Copyright (c) 2021 Valdemar Lindberg
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# Find all the tools needed for the shader compilation
FIND_PROGRAM(GLSLC glslc)
FIND_PROGRAM(GLSLLANGVALIDATOR glslangValidator)
FIND_PROGRAM(SPIRVDIS spirv-dis)
FIND_PROGRAM(SPIRVAS spirv-as)
FIND_PROGRAM(SPIRVREMAP spirv-remap)

FUNCTION(COMPILE_SHADERS source output)

	IF(GLSLC)
		FOREACH(GLSL ${GLSL_SOURCE_FILES})
		get_filename_component(FILE_NAME ${GLSL} NAME)
		SET(SPIRV_FILEPATH "{output}/${FILE_NAME}.spv")
		ADD_CUSTOM_COMMAND(
			OUTPUT ${SPIRV_FILEPATH}
			COMMAND ${CMAKE_COMMAND} -E make_directory "${EXECUTABLE_OUTPUT_PATH}/shaders/"
			COMMAND glslc ${GLSL} -o ${SPIRV_FILEPATH}
			DEPENDS ${GLSL})
		LIST(APPEND SPIRV_BINARY_FILES ${SPIRV_FILEPATH})
		ENDFOREACH(GLSL)
	ENDIF()
ENDFUNCTION()


FUNCTION(CONVERT_SHADERS source output)

ENDFUNCTION()
