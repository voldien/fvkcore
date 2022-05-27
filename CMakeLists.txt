CMAKE_MINIMUM_REQUIRED( VERSION 3.8 )
PROJECT(frag-vulkan-core)

OPTION(BUILD_SHARED_LIBS "Build package with shared libraries." ON)
IF(NOT BUILD_SHARED_LIBS)
	SET(CMAKE_EXE_LINKER_FLAGS "-static")
	SET(LINK_SEARCH_START_STATIC TRUE)
ELSE()
	MESSAGE(STATUS "Shared libraries enabled")
ENDIF(NOT BUILD_SHARED_LIBS)

############################################
# Adding directory of cmake modules.
############################################
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules" )

#############################################
#  Extract git hash and branch information.
#############################################
OPTION(IGNORE_GIT_HASH "Use Git hash" OFF)
FIND_PACKAGE(Git QUIET)
IF(NOT IGNORE_GIT_HASH)
  IF(GIT_FOUND)
    # Get the current working branch
    EXECUTE_PROCESS(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )

    # Get the latest abbreviated commit hash of the working branch
    EXECUTE_PROCESS(
    COMMAND git rev-parse --short=8 HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )

    #Extract IF there is a tag associated with current commit
    EXECUTE_PROCESS(
    COMMAND git describe --exact-match --tags 2> /dev/null || git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )
  ELSE()
    MESSAGE(FATAL_ERROR "Git must be install on the system in order to extract meta data from GIT version control" )
  ENDIF()
ENDIF()

######################################
# Finding and Target Dependencies
#####################################3
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/extern/Vulkan-Headers EXCLUDE_FROM_ALL)
IF(NOT TARGET cxxexcept)
    ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/extern/execptcxx EXCLUDE_FROM_ALL)
ENDIF()

FIND_PACKAGE(Vulkan QUIET)
IF(Vulkan_FOUND)
    MESSAGE(STATUS "Vulkan: ${Vulkan_LIBRARY}")
ENDIF()


##############################
# Target compiler flags.
#############################
IF((CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND NOT CMAKE_COMPILER_IS_MINGW)

    ADD_DEFINITIONS( -Wall -w -Wfatal-errors)

    IF (CMAKE_BUILD_TYPE STREQUAL "Release")
        MESSAGE(STATUS "Compile for release.")
        ADD_DEFINITIONS( -DNDEBUG )
        ADD_DEFINITIONS( -O2 )
        SET(CMAKE_RELEASE TRUE)
    ELSE()
        MESSAGE(STATUS "Compile for debug.")
        ADD_DEFINITIONS( -D_DEBUG )
        ADD_DEFINITIONS( -g3 -O0 )
        SET(CMAKE_DEBUG TRUE)
    ENDIF()

    #
    IF(BUILD_WITH_COVERAGE)
        MESSAGE(STATUS "Building with coverage")
        SET(GCC_COVERAGE_COMPILE_FLAGS "--coverage -fprofile-arcs -ftest-coverage")
        SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
        SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
    ENDIF()
    #
    IF(BUILD_WITH_PROFILER)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
        SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
    ENDIF()
ELSEIF(MSVC)
    # Enable multi-core compilation with MSVC
    ADD_COMPILE_OPTIONS(/MP)
ELSEIF ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
    SET(CMAKE_CXX_FLAGS "-g -fvisibility=hidden -fno-strict-aliasing -Wall -Wno-long-long ${CMAKE_CXX_FLAGS}" )
    SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
ELSEIF( CMAKE_COMPILER_IS_MINGW )
    SET(CMAKE_CXX_FLAGS "-fvisibility=hidden -fno-strict-aliasing -Wall -Wno-long-long -Wa,-mbig-obj ${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
    ADD_DEFINITIONS( -U__STRICT_ANSI__ )
ENDIF()

#####################################
# Source code files.
#####################################

SET(Vulkan_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/extern/Vulkan-Headers/include)
FILE(GLOB VKS_CORE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/fvkcore/*.cpp)
FILE(GLOB VKS_CORE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/fvkcore/*.h )

#####################################
# Main Core Library
######################################
ADD_LIBRARY(fvkcore ${VKS_CORE_SOURCE_FILES} ${VKS_CORE_HEADER_FILES})

TARGET_LINK_LIBRARIES(fvkcore PUBLIC fmt cxxexcept Vulkan-Headers ${Vulkan_LIBRARIES} )

TARGET_COMPILE_FEATURES(fvkcore PUBLIC cxx_constexpr cxx_noexcept cxx_override
	cxx_sizeof_member cxx_static_assert cxx_decltype cxx_defaulted_functions
	cxx_default_function_template_args cxx_std_17)

TARGET_INCLUDE_DIRECTORIES(fvkcore PUBLIC ${Vulkan_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/fvkcore>
	$<INSTALL_INTERFACE:fvkcore> )

SET_TARGET_PROPERTIES(fvkcore PROPERTIES
		COMPILE_FLAGS "${Vulkan_CFLAGS_OTHER}")

IF (BUILD_SHARED_LIBS AND CMAKE_SIZEOF_VOID_P EQUAL 8)
	SET_PROPERTY(TARGET fvkcore PROPERTY POSITION_INDEPENDENT_CODE ON)
ENDIF()

INSTALL(TARGETS fvkcore DESTINATION lib)

#####################
# Examples
#####################
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/examples)
