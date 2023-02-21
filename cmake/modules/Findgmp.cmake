# Try to find the GNU Multiple Precision Arithmetic Library (GMP)
# See http://gmplib.org/

if (gmp_INCLUDES AND gmp_LIBRARIES)
  set(gmp_FIND_QUIETLY TRUE)
endif (gmp_INCLUDES AND gmp_LIBRARIES)

find_path(gmp_INCLUDES
  NAMES
  gmp.h
  PATHS
  $ENV{GMPDIR}
  ${INCLUDE_INSTALL_DIR}
)

find_library(gmp_LIBRARIES gmp PATHS $ENV{GMPDIR} ${LIB_INSTALL_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gmp DEFAULT_MSG
                                  gmp_INCLUDES gmp_LIBRARIES)
mark_as_advanced(gmp_INCLUDES gmp_LIBRARIES)

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
  # Target approach
  if(NOT TARGET gmp::gmp)
    add_library(gmp::gmp INTERFACE IMPORTED)
    set_target_properties(gmp::gmp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                          "${gmp_INCLUDES}")
    set_target_properties(gmp::gmp PROPERTIES INTERFACE_LINK_LIBRARIES
                          "${gmp_LIBRARIES}")
  endif()
endif()
