# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(implot_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(implot_FRAMEWORKS_FOUND_RELEASE "${implot_FRAMEWORKS_RELEASE}" "${implot_FRAMEWORK_DIRS_RELEASE}")

set(implot_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET implot_DEPS_TARGET)
    add_library(implot_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET implot_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${implot_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${implot_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:imgui::imgui>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### implot_DEPS_TARGET to all of them
conan_package_library_targets("${implot_LIBS_RELEASE}"    # libraries
                              "${implot_LIB_DIRS_RELEASE}" # package_libdir
                              "${implot_BIN_DIRS_RELEASE}" # package_bindir
                              "${implot_LIBRARY_TYPE_RELEASE}"
                              "${implot_IS_HOST_WINDOWS_RELEASE}"
                              implot_DEPS_TARGET
                              implot_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "implot"    # package_name
                              "${implot_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${implot_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET implot::implot
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${implot_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${implot_LIBRARIES_TARGETS}>
                 )

    if("${implot_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET implot::implot
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     implot_DEPS_TARGET)
    endif()

    set_property(TARGET implot::implot
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${implot_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET implot::implot
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${implot_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET implot::implot
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${implot_LIB_DIRS_RELEASE}>)
    set_property(TARGET implot::implot
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${implot_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET implot::implot
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${implot_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(implot_LIBRARIES_RELEASE implot::implot)
