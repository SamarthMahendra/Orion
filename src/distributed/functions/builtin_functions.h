//
// Created by Samarth Mahendra on 2/28/26.
//

#ifndef BUILTIN_FUNCTIONS_H
#define BUILTIN_FUNCTIONS_H
#pragma once
#include "function_registry.h"

namespace orion::distributed {

    inline void register_builtin_functions(FunctionRegistry& registry) {

        registry.register_function("add",
            [](std::vector<std::any> args) -> std::any {
                int a = std::any_cast<int>(args[0]);
                int b = std::any_cast<int>(args[1]);
                return a + b;
            });

        registry.register_function("mul",
            [](std::vector<std::any> args) -> std::any {
                int a = std::any_cast<int>(args[0]);
                int b = std::any_cast<int>(args[1]);
                return a * b;
            });

    }

}
#endif //BUILTIN_FUNCTIONS_H
