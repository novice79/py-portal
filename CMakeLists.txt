cmake_minimum_required(VERSION 3.15)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()
project(py)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
# disable nlohmann_json warning, see:
# https://github.com/nlohmann/json/issues/1861
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wno-psabi -w")

set(uSock_path "${CMAKE_SOURCE_DIR}/3p/uWebSockets")
file(GLOB USOCKETS_FILES 
    "${uSock_path}/uSockets/src/*.c" 
    "${uSock_path}/uSockets/src/eventing/*.c" 
    "${uSock_path}/uSockets/src/crypto/*.c" 
    "${uSock_path}/uSockets/src/crypto/sni_tree.cpp"
    "${uSock_path}/uSockets/src/crypto/openssl.cpp"
)
add_library(uSockets STATIC ${USOCKETS_FILES})
target_compile_definitions(uSockets PRIVATE LIBUS_USE_OPENSSL)
target_include_directories(uSockets PUBLIC "${uSock_path}/uSockets/src")
include_directories("${uSock_path}/src")

file(GLOB SOURCE_FILES 
    "cpp/*.c" 
    "cpp/*.cpp" 
    "cpp/*.h" 
    "cpp/*.hpp"
)
# or uWS::Loop::get()->setSilent(true);
add_compile_definitions(UWS_HTTPRESPONSE_NO_WRITEMARK FOR_DOCKER)
# or add_definitions(-DFOO -DBAR ...)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} uSockets)
add_custom_command(TARGET ${PROJECT_NAME}
POST_BUILD
COMMAND ${CMAKE_STRIP} ${PROJECT_NAME} -o "${PROJECT_NAME}_striped"
COMMENT "strip file"
)
set(HEADER_DIR 
    "${CMAKE_SOURCE_DIR}/3p/Simple-Web-Server"
    "${CMAKE_SOURCE_DIR}/3p/Simple-WebSocket-Server"   
)
target_include_directories(${PROJECT_NAME} PRIVATE ${HEADER_DIR})
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory("${CMAKE_SOURCE_DIR}/3p/nlohmann_json")
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
# set(Boost_USE_STATIC_RUNTIME     ON)
if(DEFINED ENV{VCPKG_ROOT})
    find_package(Boost COMPONENTS date_time filesystem atomic log system REQUIRED)
else()
    find_package(Boost COMPONENTS date_time filesystem atomic log_setup log system REQUIRED)
endif()
include_directories(${Boost_INCLUDE_DIRS})
find_library(libtorrent torrent)
find_library(libtry_signal try_signal)
message("Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}")
find_package(ZLIB REQUIRED)
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
message("OPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}")
if(DEFINED ENV{VCPKG_ROOT})
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} unofficial::sqlite3::sqlite3)
else()
    find_package (SQLite3 REQUIRED)
    include_directories(${SQLite3_INCLUDE_DIRS})
    message("SQLite3_INCLUDE_DIRS=${SQLite3_INCLUDE_DIRS}")
    message("SQLite3_LIBRARIES=${SQLite3_LIBRARIES}")
    target_link_libraries (${PROJECT_NAME} ${SQLite3_LIBRARIES})
endif()
find_library(libmagic magic)
target_link_libraries(${PROJECT_NAME} ${libtorrent} ${libtry_signal})
target_link_libraries(${PROJECT_NAME} ${libmagic} )
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} )
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7-a")
    message("build for raspberry pi, link static libc++")
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} -static -static-libstdc++ -static-libgcc)
endif()
# ###########################3
# add_executable(test test/test.cpp)
# target_include_directories(test PRIVATE ${HEADER_DIR})
# target_link_libraries(test ${Boost_LIBRARIES} )
# target_link_libraries(test OpenSSL::SSL OpenSSL::Crypto)
# target_link_libraries(test ZLIB::ZLIB)
# target_link_libraries(test ${CMAKE_DL_LIBS} -static -static-libstdc++ -static-libgcc)
# #############################