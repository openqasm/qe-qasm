# Try to find the MPFR library
# See http://www.mpfr.org/
#
# This module supports requiring a minimum version, e.g. you can do
#   find_package(mpfr 2.3.0)
# to require version 2.3.0 to newer of MPFR.
#
# Once done this will define
#
#  mpfr_FOUND - system has MPFR lib with correct version
#  mpfr_INCLUDES - the MPFR include directory
#  mpfr_LIBRARIES - the MPFR library
#  mpfr_VERSION - MPFR version

# Copyright (c) 2006, 2007 Montel Laurent, <montel@kde.org>
# Copyright (c) 2008, 2009 Gael Guennebaud, <g.gael@free.fr>
# Copyright (c) 2010 Jitse Niesen, <jitse@maths.leeds.ac.uk>
# Redistribution and use is allowed according to the terms of the BSD license.

# Set mpfr_INCLUDES

find_path(mpfr_INCLUDES
  NAMES
  mpfr.h
  PATHS
  $ENV{GMPDIR}
  ${INCLUDE_INSTALL_DIR}
)

# Set mpfr_FIND_VERSION to 1.0.0 if no minimum version is specified

if(NOT mpfr_FIND_VERSION)
  if(NOT mpfr_FIND_VERSION_MAJOR)
    set(mpfr_FIND_VERSION_MAJOR 1)
  endif(NOT mpfr_FIND_VERSION_MAJOR)
  if(NOT mpfr_FIND_VERSION_MINOR)
    set(mpfr_FIND_VERSION_MINOR 0)
  endif(NOT mpfr_FIND_VERSION_MINOR)
  if(NOT mpfr_FIND_VERSION_PATCH)
    set(mpfr_FIND_VERSION_PATCH 0)
  endif(NOT mpfr_FIND_VERSION_PATCH)

  set(mpfr_FIND_VERSION "${mpfr_FIND_VERSION_MAJOR}.${mpfr_FIND_VERSION_MINOR}.${mpfr_FIND_VERSION_PATCH}")
endif(NOT mpfr_FIND_VERSION)


if(mpfr_INCLUDES)
  # Set mpfr_VERSION
  file(READ "${mpfr_INCLUDES}/mpfr.h" _mpfr_version_header)
  string(REGEX MATCH "define[ \t]+MPFR_VERSION_MAJOR[ \t]+([0-9]+)" _mpfr_major_version_match "${_mpfr_version_header}")
  set(mpfr_MAJOR_VERSION "${CMAKE_MATCH_1}")
  string(REGEX MATCH "define[ \t]+MPFR_VERSION_MINOR[ \t]+([0-9]+)" _mpfr_minor_version_match "${_mpfr_version_header}")
  set(mpfr_MINOR_VERSION "${CMAKE_MATCH_1}")
  string(REGEX MATCH "define[ \t]+MPFR_VERSION_PATCHLEVEL[ \t]+([0-9]+)" _mpfr_patchlevel_version_match "${_mpfr_version_header}")
  set(mpfr_PATCHLEVEL_VERSION "${CMAKE_MATCH_1}")
  set(mpfr_VERSION ${mpfr_MAJOR_VERSION}.${mpfr_MINOR_VERSION}.${mpfr_PATCHLEVEL_VERSION})

# Check whether found version exceeds minimum version
  if(${mpfr_VERSION} VERSION_LESS ${mpfr_FIND_VERSION})
    set(mpfr_VERSION_OK FALSE)
    message(STATUS "MPFR version ${mpfr_VERSION} found in ${mpfr_INCLUDES}, "
                   "but at least version ${mpfr_FIND_VERSION} is required")
  else(${mpfr_VERSION} VERSION_LESS ${mpfr_FIND_VERSION})
    set(mpfr_VERSION_OK TRUE)
  endif(${mpfr_VERSION} VERSION_LESS ${mpfr_FIND_VERSION})

endif(mpfr_INCLUDES)

# Set mpfr_LIBRARIES
find_library(mpfr_LIBRARIES mpfr PATHS $ENV{GMPDIR} ${LIB_INSTALL_DIR})

# Epilogue
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mpfr DEFAULT_MSG
                                  mpfr_INCLUDES mpfr_LIBRARIES mpfr_VERSION_OK)
mark_as_advanced(mpfr_INCLUDES mpfr_LIBRARIES)

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
    # Target approach
    if(NOT TARGET mpfr::mpfr)
        add_library(mpfr::mpfr INTERFACE IMPORTED)
        set_property(TARGET mpfr::mpfr PROPERTY INTERFACE_LINK_LIBRARIES
                     "${mpfr_LIBRARIES_TARGETS};${mpfr_LINKER_FLAGS_LIST}")
    endif()
endif()
