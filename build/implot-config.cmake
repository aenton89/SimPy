########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(implot_FIND_QUIETLY)
    set(implot_MESSAGE_MODE VERBOSE)
else()
    set(implot_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/implotTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${implot_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(implot_VERSION_STRING "0.16")
set(implot_INCLUDE_DIRS ${implot_INCLUDE_DIRS_RELEASE} )
set(implot_INCLUDE_DIR ${implot_INCLUDE_DIRS_RELEASE} )
set(implot_LIBRARIES ${implot_LIBRARIES_RELEASE} )
set(implot_DEFINITIONS ${implot_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${implot_BUILD_MODULES_PATHS_RELEASE} )
    message(${implot_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


