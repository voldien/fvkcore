CMAKE_MINIMUM_REQUIRED( VERSION 3.8 )
PROJECT(frag-vulkan-core)

OPTION(BUILD_SHARED_LIBS "Build package with shared libraries." ON)
IF(NOT BUILD_SHARED_LIBS)
	SET(CMAKE_EXE_LINKER_FLAGS "-static")
	SET(LINK_SEARCH_START_STATIC TRUE)
ELSE()
	MESSAGE(STATUS "Shared libraries enabled")
ENDIF(NOT BUILD_SHARED_LIBS)

ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/extern/Vulkan-Headers EXCLUDE_FROM_ALL)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/extern/execptcxx EXCLUDE_FROM_ALL)
SET(Vulkan_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/extern/Vulkan-Headers/include)

FILE(GLOB VKS_CORE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp)
FILE(GLOB VKS_CORE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h )

ADD_LIBRARY(vkcommon-core ${VKS_CORE_SOURCE_FILES} ${VKS_CORE_HEADER_FILES})

TARGET_LINK_LIBRARIES(vkcommon-core PRIVATE cxxexcept ${Vulkan_LIBRARIES} fmt Vulkan-Headers)

TARGET_COMPILE_FEATURES(vkcommon-core PUBLIC cxx_constexpr cxx_noexcept cxx_override
	cxx_sizeof_member cxx_static_assert cxx_decltype cxx_defaulted_functions
	cxx_default_function_template_args cxx_std_17)

TARGET_INCLUDE_DIRECTORIES(vkcommon-core PUBLIC ${Vulkan_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Core>
	$<INSTALL_INTERFACE:/Core> )


INSTALL(TARGETS vkcommon-core DESTINATION lib)
