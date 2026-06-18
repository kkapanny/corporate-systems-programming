include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

set(TESTS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(warehouse_unit_tests
    unit/test_product.cpp
    unit/test_order.cpp
    unit/test_transaction.cpp
    unit/test_memory_store.cpp
    unit/test_inventory_facade.cpp
)

target_include_directories(warehouse_unit_tests PRIVATE "${TESTS_INCLUDE_DIR}")

target_link_libraries(warehouse_unit_tests
    PRIVATE
    warehouse_lib
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(warehouse_unit_tests)

function(add_scenario_executable target_name source_file)
    add_executable(${target_name} ${source_file})
    target_include_directories(${target_name} PRIVATE "${TESTS_INCLUDE_DIR}")
    target_link_libraries(${target_name} PRIVATE warehouse_lib)
endfunction()

add_scenario_executable(scenario_01_add_product scenarios/scenario_01_add_product.cpp)
add_scenario_executable(scenario_02_list_products scenarios/scenario_02_list_products.cpp)
add_scenario_executable(scenario_03_income_flow scenarios/scenario_03_income_flow.cpp)
add_scenario_executable(scenario_04_expense_flow scenarios/scenario_04_expense_flow.cpp)
add_scenario_executable(scenario_05_replenishment_draft scenarios/scenario_05_replenishment_draft.cpp)
add_scenario_executable(scenario_06_full_cycle scenarios/scenario_06_full_cycle.cpp)
add_scenario_executable(scenario_07_error_handling scenarios/scenario_07_error_handling.cpp)
