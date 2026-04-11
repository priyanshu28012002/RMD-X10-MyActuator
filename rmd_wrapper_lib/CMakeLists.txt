cmake_minimum_required(VERSION 3.10)
project(librmd VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Library ----
add_library(rmd SHARED
    src/SerialPort.cpp
    src/SerialComm.cpp
    src/x10_api_base.cpp
    src/x10_api.cpp
    src/MyActuator.cpp
   
)

target_include_directories(rmd
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(rmd PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

install(TARGETS rmd
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
)



add_subdirectory(example)

# sudo make install
# sudo ldconfig