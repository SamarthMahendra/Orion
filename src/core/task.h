#pragma once

#include <string>
#include <vector>
#include <functional>
#include <any>

#include "object_ref.h"

namespace orion {

    struct Task {
        std::string id;
        std::vector<ObjectRef> deps;

        // ALWAYS takes dependency values
        std::function<std::any(std::vector<std::any>)> work;

        // Task with deps
        Task(std::string id,
             std::vector<ObjectRef> deps,
             std::function<std::any(std::vector<std::any>)> fn)
            : id(std::move(id)),
              deps(std::move(deps)),
              work(std::move(fn)) {}

        // Task without deps
        Task(std::string id,
             std::vector<ObjectRef> deps,
             std::function<std::any()> fn)
            : id(std::move(id)),
              deps(std::move(deps)),
              work([fn = std::move(fn)](std::vector<std::any>) {
                    return fn();
                }) {}
    };

} // namespace orion
