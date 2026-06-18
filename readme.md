## Локальная сборка

```bash
cmake -S . -B build
cmake --build build
```

Запуск приложения:

```bash
./build/warehouse
```

## Тесты

```bash
# сборка
cmake -S . -B build
cmake --build build

# test
ctest --test-dir build --output-on-failure
# or
./build/tests/warehouse_unit_tests

# сценарии
./build/tests/scenario_01_add_product
./build/tests/scenario_02_list_products
./build/tests/scenario_03_income_flow
./build/tests/scenario_04_expense_flow
./build/tests/scenario_05_replenishment_draft
./build/tests/scenario_06_full_cycle
./build/tests/scenario_07_error_handling
```

## Docker

```bash
docker build -t warehouse-mgmt:latest . # сборка
```

```bash
`docker run -it warehouse-mgmt:latest`              # запуск приложения 
  
docker run --rm warehouse-mgmt:latest test          # Unit-тесты   
docker run --rm warehouse-mgmt:latest scenarios     # все 7 сценарных тестов  
docker run --rm warehouse-mgmt:latest scenario 03   # один сценарий   
```

