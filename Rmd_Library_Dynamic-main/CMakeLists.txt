cmake_minimum_required(VERSION 3.10)
project(Rmd_Library)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-fPIC)


# Include the directories where the headers are located
include_directories(
    "${CMAKE_SOURCE_DIR}/CppLinuxSerial"
    "${CMAKE_SOURCE_DIR}/include"
)

# Load CPPLinuxSerial library
# set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

set(HEADER_FILES
    include/crc16.h
    # include/serial_comm.h 
    include/x10_api_base.h
    include/x10_api.h 
    include/x10_reg.h 
)

# Add the source files
set(SOURCE_FILES
    # src/serial_comm.cpp 
    src/x10_api_base.cpp
    src/x10_api.cpp 
     
)

include_directories(include)

# Create a Dynamic library
add_library(Rmd_Library SHARED ${SOURCE_FILES} ${HEADER_FILES})

# Link CPPLinuxSerial library
# target_link_libraries(Rmd_Library PRIVATE CppLinuxSerial)

# Add option to build examples
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
 add_subdirectory(examples)
endif()

