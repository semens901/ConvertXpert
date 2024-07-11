cmake_minimum_required(VERSION 3.5)

project(ConvertXpert LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CXP_INCLUDE_FILES core/Converter.h)
set(CXP_SOURCE_FILES core/Converter.cpp)
add_executable(ConvertXpert core/main.cpp ${CXP_SOURCE_FILES} ${CXP_INCLUDE_FILES})

include(GNUInstallDirs)
install(TARGETS ConvertXpert
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
