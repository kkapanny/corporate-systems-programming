FROM ubuntu:24.04 AS builder

# отключение интерактивных запросов при установке пакетов
ENV DEBIAN_FRONTEND=noninteractive

# установка пакетов для сборки
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    git \
    ninja-build \
    && rm -rf /var/lib/apt/lists/* # очистка списка пакетов

# переход в рабочую директорию
WORKDIR /src

# копирование исходников по отдельным путям
COPY CMakeLists.txt .
COPY cmake/ cmake/
COPY include/ include/
COPY src/ src/
COPY tests/ tests/

# сборка проекта (с тестами)
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel \
    && ctest --test-dir build --output-on-failure

# образ только для запуска готовых программ
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

# установка библиотеки C++
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# копирование готовых бинарников из стадии builder
COPY --from=builder /src/build/warehouse /app/warehouse
COPY --from=builder /src/build/tests/warehouse_unit_tests /app/tests/
COPY --from=builder /src/build/tests/scenario_01_add_product /app/tests/
COPY --from=builder /src/build/tests/scenario_02_list_products /app/tests/
COPY --from=builder /src/build/tests/scenario_03_income_flow /app/tests/
COPY --from=builder /src/build/tests/scenario_04_expense_flow /app/tests/
COPY --from=builder /src/build/tests/scenario_05_replenishment_draft /app/tests/
COPY --from=builder /src/build/tests/scenario_06_full_cycle /app/tests/
COPY --from=builder /src/build/tests/scenario_07_error_handling /app/tests/

# копирование скрипта-обёртки
COPY docker/docker-entrypoint.sh /app/docker-entrypoint.sh
RUN chmod +x /app/docker-entrypoint.sh

# запуск скрипта-обёртки
ENTRYPOINT ["/app/docker-entrypoint.sh"]
CMD ["warehouse"]
