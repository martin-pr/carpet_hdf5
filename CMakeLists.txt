project (group_motion_editing)

# set(CMAKE_CXX_COMPILER "clang")

cmake_minimum_required (VERSION 2.8.4)
# set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(LIBS "")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_definitions(-Wall -Werror -std=c++11 -g)
#add_definitions(-Wall -Werror -std=c++98 -g)

###########################################################
# DEPENDENCIES

# looking for boost
find_package(Boost REQUIRED COMPONENTS filesystem system serialization unit_test_framework program_options regex thread)
include_directories(${Boost_INCLUDE_DIR})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

# looking for openexr libs
SET(OPENEXR_LIBRARY_PATHS /usr/lib /usr/local/lib /sw/lib /opt/local/lib $ENV{PROGRAM_FILES}/OpenEXR/lib/static)
FIND_PATH(OPENEXR_INCLUDE_DIR ImfRgbaFile.h PATH_SUFFIXES OpenEXR /usr/include /usr/local/include /sw/include /opt/local/include)
FIND_LIBRARY(OPENEXR_HALF_LIBRARY NAMES Half PATHS ${OPENEXR_LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_IEX_LIBRARY NAMES Iex PATHS ${OPENEXR_LIBRARY_PATHS})
if (OPENEXR_INCLUDE_DIR AND OPENEXR_HALF_LIBRARY AND OPENEXR_IEX_LIBRARY)
  set (HDF5_FOUND TRUE)
  message (STATUS "OpenEXR library found (${OPENEXR_INCLUDE_DIR}, ${OPENEXR_IEX_LIBRARY} and ${OPENEXR_HALF_LIBRARY}).")
else (OPENEXR_INCLUDE_DIR AND OPENEXR_HALF_LIBRARY AND OPENEXR_IEX_LIBRARY)
  message (FATAL_ERROR "OpenEXR library not found (${OPENEXR_INCLUDE_DIR}, ${OPENEXR_IEX_LIBRARY} and ${OPENEXR_HALF_LIBRARY}).")
endif (OPENEXR_INCLUDE_DIR AND OPENEXR_HALF_LIBRARY AND OPENEXR_IEX_LIBRARY)
include_directories(${OPENEXR_INCLUDE_DIR})
set(LIBS ${LIBS} ${OPENEXR_HALF_LIBRARY} ${OPENEXR_IEX_LIBRARY})

# looking for hdf5 libs
SET(HDF5_LIBRARY_PATHS /usr/lib /usr/local/lib /sw/lib /opt/local/lib /usr/lib/x86_64-linux-gnu /usr/local/HDF_Group/HDF5/1.8.16/lib)
FIND_PATH(HDF5_INCLUDE_DIR hdf5.h /usr/include/hdf5/serial /usr/local/include/hdf5/serial /sw/include /opt/local/include /usr/local/HDF_Group/HDF5/1.8.16/include)
FIND_LIBRARY(HDF5_LIBRARY NAMES hdf5 hdf5-shared PATHS ${HDF5_LIBRARY_PATHS})
FIND_LIBRARY(HDF5_CPP_LIBRARY NAMES hdf5_cpp hdf5_cpp-shared PATHS ${HDF5_LIBRARY_PATHS})
if (HDF5_INCLUDE_DIR AND HDF5_LIBRARY AND HDF5_CPP_LIBRARY)
  set (HDF5_FOUND TRUE)
  message (STATUS "HDF5 library found (${HDF5_INCLUDE_DIR}, ${HDF5_LIBRARY}, ${HDF5_CPP_LIBRARY}).")
else (HDF5_INCLUDE_DIR AND HDF5_LIBRARY AND HDF5_CPP_LIBRARY)
  message (FATAL_ERROR "HDF5 library not found (${HDF5_INCLUDE_DIR}, ${HDF5_LIBRARY}, ${HDF5_CPP_LIBRARY}).")
endif (HDF5_INCLUDE_DIR AND HDF5_LIBRARY AND HDF5_CPP_LIBRARY)
include_directories(${HDF5_INCLUDE_DIR})
set(LIBS ${LIBS} ${HDF5_LIBRARY} ${HDF5_CPP_LIBRARY})

# blindly adding tbb
# set(LIBS ${LIBS} tbb)

###########################################################
# BUILD

include_directories(src)
add_subdirectory (src)
