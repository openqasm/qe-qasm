# This module supports requiring a minimum version, e.g. you can do
#   find_package(mpc 1.2.0)
# to require version 1.2.0 to newer of MPC.
#
# Once done this will define
#
#  mpc_FOUND - system has MPC lib with correct version
#  mpc_INCLUDES - the MPC include directory
#  mpc_LIBRARIES - the MPC library
#  mpc_VERSION - MPC version
if (mpc_INCLUDES AND mpc_LIBRARIES)
    set(mpc_FIND_QUIETLY TRUE)
endif (mpc_INCLUDES AND mpc_LIBRARIES)

find_path(mpc_INCLUDES
        NAMES
        mpc.h
        PATHS
        $ENV{GMPDIR}
        ${INCLUDE_INSTALL_DIR}
        )

# Set mpc_FIND_VERSION to 1.0.0 if no minimum version is specified

if(NOT mpc_FIND_VERSION)
    if(NOT mpc_FIND_VERSION_MAJOR)
        set(mpc_FIND_VERSION_MAJOR 1)
    endif(NOT mpc_FIND_VERSION_MAJOR)
    if(NOT mpc_FIND_VERSION_MINOR)
        set(mpc_FIND_VERSION_MINOR 0)
    endif(NOT mpc_FIND_VERSION_MINOR)
    if(NOT mpc_FIND_VERSION_PATCH)
        set(mpc_FIND_VERSION_PATCH 0)
    endif(NOT mpc_FIND_VERSION_PATCH)

    set(mpc_FIND_VERSION "${mpc_FIND_VERSION_MAJOR}.${mpc_FIND_VERSION_MINOR}.${mpc_FIND_VERSION_PATCH}")
endif(NOT mpc_FIND_VERSION)

if(mpc_INCLUDES)
    # Set mpc_VERSION
    file(READ "${mpc_INCLUDES}/mpc.h" _mpc_version_header)
    string(REGEX MATCH "define[ \t]+MPC_VERSION_MAJOR[ \t]+([0-9]+)" _mpc_major_version_match "${_mpc_version_header}")
    set(mpc_MAJOR_VERSION "${CMAKE_MATCH_1}")
    string(REGEX MATCH "define[ \t]+MPC_VERSION_MINOR[ \t]+([0-9]+)" _mpc_minor_version_match "${_mpc_version_header}")
    set(mpc_MINOR_VERSION "${CMAKE_MATCH_1}")
    string(REGEX MATCH "define[ \t]+MPC_VERSION_PATCHLEVEL[ \t]+([0-9]+)" _mpc_patchlevel_version_match "${_mpc_version_header}")
    set(mpc_PATCHLEVEL_VERSION "${CMAKE_MATCH_1}")
    set(mpc_VERSION ${mpc_MAJOR_VERSION}.${mpc_MINOR_VERSION}.${mpc_PATCHLEVEL_VERSION})

    # Check whether found version exceeds minimum version
    if(${mpc_VERSION} VERSION_LESS ${mpc_FIND_VERSION})
        set(mpc_VERSION_OK FALSE)
        message(STATUS "MPC version ${mpc_VERSION} found in ${mpc_INCLUDES}, "
                "but at least version ${mpc_FIND_VERSION} is required")
    else(${mpc_VERSION} VERSION_LESS ${mpc_FIND_VERSION})
        set(mpc_VERSION_OK TRUE)
    endif(${mpc_VERSION} VERSION_LESS ${mpc_FIND_VERSION})

endif(mpc_INCLUDES)

find_library(mpc_LIBRARIES mpc PATHS $ENV{MPCDIR} ${LIB_INSTALL_DIR})

# Epilogue
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mpc DEFAULT_MSG
        mpc_INCLUDES mpc_LIBRARIES mpc_VERSION_OK)
mark_as_advanced(mpc_INCLUDES mpc_LIBRARIES)

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
  # Target approach
  if(NOT TARGET mpc::mpc)
    add_library(mpc::mpc INTERFACE IMPORTED)
    set_target_properties(mpc::mpc PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                          "${mpc_INCLUDES}")
    set_property(TARGET mpc::mpc PROPERTY INTERFACE_LINK_LIBRARIES
                "${mpc_LIBRARIES}")

    # Library dependencies
    include(CMakeFindDependencyMacro)

    if(NOT mpfr_FOUND)
        find_dependency(mpfr REQUIRED)
    else()
        message(STATUS "Dependency mpfr already found")
    endif()
  endif()
endif()
