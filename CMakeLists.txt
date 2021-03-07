cmake_minimum_required(VERSION 3.15)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()
project(py)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -w")

file(GLOB SOURCE_FILES 
    "cpp/*.c" 
    "cpp/*.cpp" 
    "cpp/*.h" 
    "cpp/*.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

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
find_package(Boost COMPONENTS date_time filesystem atomic log system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
message("Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}")
find_package(ZLIB REQUIRED)
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
message("OPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}")
if(DEFINED ENV{VCPKG_ROOT})
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} unofficial::sqlite3::sqlite3)
else()
    find_package (SQLite3)
    include_directories(${SQLITE3_INCLUDE_DIRS})
    target_link_libraries (${PROJECT_NAME} ${SQLITE3_LIBRARIES})
endif()
find_library(libmagic magic)
target_link_libraries(${PROJECT_NAME} ${libmagic} )
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} )
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)

target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})

