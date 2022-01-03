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
FIND_PROGRAM(SPIRVCROSS spirv-cross)


#TODO add working directory
FUNCTION(COMPILE_SHADERS source output)

	IF(GLSLC)
		FOREACH(GLSL ${source})
		GET_FILENAME_COMPONENT(FILE_NAME ${GLSL} NAME)
		SET(SPIRV_FILEPATH "${output}/${FILE_NAME}.spv")
		ADD_CUSTOM_COMMAND(
			OUTPUT ${SPIRV_FILEPATH}
			COMMAND ${CMAKE_COMMAND} -E make_directory "${output}"
			COMMAND ${GLSLC} -O0 -g ${GLSL} -o ${SPIRV_FILEPATH}
			VERBATIM
			DEPENDS ${GLSL})
		LIST(APPEND SPIRV_BINARY_FILES ${SPIRV_FILEPATH})
		ENDFOREACH(GLSL)
		ADD_CUSTOM_TARGET(
			Shaders
			DEPENDS ${SPIRV_BINARY_FILES}
			)
	ELSE()
		MESSAGE(WARNING "Could not find glslc")
	ENDIF()
ENDFUNCTION()

#TODO add working directory
FUNCTION(CONVERT_SPIRV_2_GLSL source output)
	IF(SPIRVCROSS)
		FOREACH(SPIRV ${source})
		GET_FILENAME_COMPONENT(FILE_NAME ${GLSL} NAME)
		SET(GLSL_FILEPATH "${output}/${FILE_NAME}.glsl")
		ADD_CUSTOM_COMMAND(
				OUTPUT ${GLSL_FILEPATH}
				COMMAND ${CMAKE_COMMAND} -E make_directory "${output}"
				COMMAND ${SPIRVCROSS} --version 310 --es ${SPIRV} --output ${GLSL_FILEPATH} --force-temporary
				VERBATIM
				DEPENDS ${SPIRV})
		LIST(APPEND GLSL_SOURCE_FILES ${GLSL_FILEPATH})
		ENDFOREACH(SPIRV)
	ELSE()
		MESSAGE(WARNING "Could not find spirv-cross")
	ENDIF()
ENDFUNCTION()
