//
// Created by Samarth Mahendra on 2/28/26.
//

#ifndef FUNCTION_REGISTRY_H
#define FUNCTION_REGISTRY_H



#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <any>

namespace orion::distributed {

    class FunctionRegistry {
    public:
        using Func = std::function<std::any(std::vector<std::any>)>;

        void register_function(const std::string& name, Func fn);

        bool exists(const std::string& name) const;

        std::any invoke(const std::string& name,
                        std::vector<std::any> args);

    private:
        std::unordered_map<std::string, Func> functions_;
    };

}


#endif //FUNCTION_REGISTRY_H
