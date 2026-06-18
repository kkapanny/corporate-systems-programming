#!/bin/sh
set -e

APP_DIR=/app
TESTS_DIR="${APP_DIR}/tests"

run_unit_tests() {
    echo "=== Catch2 unit tests (warehouse_unit_tests) ==="
    exec "${TESTS_DIR}/warehouse_unit_tests" "$@"
}

run_all_scenarios() {
    failed=0
    for exe in \
        scenario_01_add_product \
        scenario_02_list_products \
        scenario_03_income_flow \
        scenario_04_expense_flow \
        scenario_05_replenishment_draft \
        scenario_06_full_cycle \
        scenario_07_error_handling
    do
        echo "=== scenario: ${exe} ==="
        if ! "${TESTS_DIR}/${exe}"; then
            failed=1
        fi
    done
    exit "${failed}"
}

run_scenario() {
    name="$1"
    shift
  case "${name}" in
        01|1) exe=scenario_01_add_product ;;
        02|2) exe=scenario_02_list_products ;;
        03|3) exe=scenario_03_income_flow ;;
        04|4) exe=scenario_04_expense_flow ;;
        05|5) exe=scenario_05_replenishment_draft ;;
        06|6) exe=scenario_06_full_cycle ;;
        07|7) exe=scenario_07_error_handling ;;
        *)
            echo "Unknown scenario: ${name}" >&2
            echo "Use: scenario <01-07> or scenario_<name>" >&2
            exit 1
            ;;
    esac
    exec "${TESTS_DIR}/${exe}" "$@"
}

usage() {
    cat <<'EOF'
warehouse_mgmt container

Usage:
  docker run -it <image>                    # интерактивное CLI-приложение warehouse
  docker run --rm <image> test              # все unit-тесты (Catch2)
  docker run --rm <image> test --help       # аргументы передаются в Catch2
  docker run --rm <image> scenarios         # все сценарные тесты
  docker run --rm <image> scenario 03       # один сценарий (01–07)
  docker run --rm <image> warehouse           # явный запуск приложения

Передача ввода в CLI (пример):
  printf '2\n0\n' | docker run -i --rm <image>

EOF
}

cmd="${1:-warehouse}"
shift || true

case "${cmd}" in
    warehouse|app)
        exec "${APP_DIR}/warehouse" "$@"
        ;;
    test|tests|unit)
        run_unit_tests "$@"
        ;;
    scenarios|scenario-all)
        run_all_scenarios
        ;;
    scenario)
        run_scenario "$@"
        ;;
    help|-h|--help)
        usage
        exit 0
        ;;
    *)
        echo "Unknown command: ${cmd}" >&2
        usage
        exit 1
        ;;
esac
