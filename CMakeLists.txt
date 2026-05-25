cmake_minimum_required(VERSION 3.14)

project(warehouse_mgmt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(warehouse_lib STATIC
    src/product.cpp
    src/transaction.cpp
    src/order.cpp
    src/memory_store.cpp
    src/inventory_facade.cpp
)

target_include_directories(warehouse_lib
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(warehouse src/main.cpp)
target_link_libraries(warehouse PRIVATE warehouse_lib)

enable_testing()
add_subdirectory(tests)
