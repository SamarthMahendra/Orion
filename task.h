#pragma once
#ifndef TASK_H
#define TASK_H

#endif //TASK_H

#include <string>
#include <functional>
#include <any>
#include <utility>
#include "object_ref.h"

// PURPOSE:
// --------
// Defines the fundamental unit of execution in Orion.
// A Task represents "something that can be executed by a worker".
//
// This is the smallest abstraction in the system.
// Everything else (scheduler, workers, object store) exists to move Tasks around.
//
// ----------------------------
// 1. A unique identifier for each task (TaskId).
// 2. A way to represent executable work (initially a callable).
// 3. (Later) Metadata such as dependencies, status, or resource hints.
//
// - Keep this simple.
// - Start with a fixed return type (e.g., int or std::any).
// - This file must NOT know about workers or schedulers.

namespace orion {

    struct Task {
        std::string id;
        std::function<std::any()> work;
        std::vector<ObjectRef> deps;


        // Accept ANY callable that returns ANY type,
        // and store it as a std::function<std::any()>.
        // type erasure.
        // F&& f (universal / forwarding reference)
        // [fn = std::forward<F>(f)] Create a new lambda that owns the callable.
        template<typename F>
        Task(std::string id,
             std::vector<ObjectRef> deps,
             F&& f)
            : id(std::move(id)),
              deps(std::move(deps)),
              work([fn = std::forward<F>(f)]() -> std::any {
                  return fn();
              }) {}
    };

} // namespace orion

