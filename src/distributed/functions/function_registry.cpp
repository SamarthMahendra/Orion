//
// Created by Samarth Mahendra on 2/28/26.
//

#include "function_registry.h"
#include "function_registry.h"
#include <stdexcept>

namespace orion::distributed {

    void FunctionRegistry::register_function(
        const std::string& name,
        Func fn)
    {
        functions_[name] = std::move(fn);
    }

    bool FunctionRegistry::exists(const std::string& name) const {
        return functions_.find(name) != functions_.end();
    }

    std::any FunctionRegistry::invoke(
        const std::string& name,
        std::vector<std::any> args)
    {
        auto it = functions_.find(name);
        if (it == functions_.end())
            throw std::runtime_error("Function not found: " + name);

        return it->second(std::move(args));
    }

}