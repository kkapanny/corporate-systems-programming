#pragma once

#include <iostream>
#include <string>

inline int scenario_fail(const std::string& message) {
    std::cerr << "[FAIL] " << message << '\n';
    return 1;
}

inline int scenario_pass(const std::string& message) {
    std::cout << "[OK] " << message << '\n';
    return 0;
}
