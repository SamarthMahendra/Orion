//
// Created by Samarth Mahendra on 2/27/26.
//

#include "node_runtime.h"

#include <iostream>

namespace orion::distributed {

    NodeRuntime::NodeRuntime(size_t num_workers, int port)
        : num_workers_(num_workers), port_(port) {}

    // Start local runtime
    void NodeRuntime::start() {
        if (running_) return;

        std::cout << "[NodeRuntime] Starting node on port " << port_ << "\n";

        runtime_ = std::make_unique<orion::Runtime>(num_workers_);

        // Later:
        // start RPC server here

        running_ = true;
    }

    // Stop everything
    void NodeRuntime::stop() {
        if (!running_) return;

        std::cout << "[NodeRuntime] Shutting down node\n";

        if (runtime_) {
            runtime_->shutdown();
            runtime_.reset();
        }

        // Later:
        // stop RPC server here

        running_ = false;
    }

    orion::Runtime& NodeRuntime::local_runtime() {
        return *runtime_;
    }

} // namespace orion::distributed