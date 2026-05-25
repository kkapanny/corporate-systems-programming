сборка:

```bash
cmake -S . -B build
cmake --build build
```

запуск:

```bash
cd build
./warehouse
```



сборка и запуск тестов:

```cmake -S . -B build
cmake --build build

# unit-тесты
./build/tests/warehouse_unit_tests

# сценарии по отдельности
./build/tests/scenario_01_add_product
./build/tests/scenario_02_list_products
./build/tests/scenario_03_income_flow
./build/tests/scenario_04_expense_flow
./build/tests/scenario_05_replenishment_draft
./build/tests/scenario_06_full_cycle
./build/tests/scenario_07_error_handling
```

