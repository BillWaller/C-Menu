#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cm" for configuration "Release"
set_property(TARGET cm APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cm PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libcm.so.0.2.9"
  IMPORTED_SONAME_RELEASE "libcm.so.0"
  )

list(APPEND _cmake_import_check_targets cm )
list(APPEND _cmake_import_check_files_for_cm "${_IMPORT_PREFIX}/lib64/libcm.so.0.2.9" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
