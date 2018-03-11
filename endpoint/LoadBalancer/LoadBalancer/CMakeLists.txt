cmake_minimum_required(VERSION 3.0.0)
project (endpoint)
set(CMAKE_CXX_COMPILER gcc-7.2.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.62.0)
#find_package(Boost REQUIRED thread)

if(Boost_FOUND)
    message ("boost found")
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message (FATAL_ERROR "Cannot find Boost")
endif()


include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)


ExternalProject_Add(amqp
    GIT_REPOSITORY https://github.com/CopernicaMarketingSoftware/AMQP-CPP
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)

ExternalProject_Get_Property(amqp install_dir)
include_directories(${install_dir}/include)
include_directories(../../../commonCPP)
include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

MESSAGE(${EXTERNAL_INSTALL_LOCATION})

add_executable(endpoint amqp_client.cpp load_balancer.cpp socket_acceptor.cpp socket_acceptor_manage_io_service.cpp socket_acceptor_nodes.cpp socket_acceptor_requests.cpp socket_acceptor_requests_with_ascii_header.cpp stdafx.cpp )
add_dependencies(endpoint amqp)

target_link_libraries(endpoint pthread ${EXTERNAL_INSTALL_LOCATION}/lib/libamqpcpp.a)
