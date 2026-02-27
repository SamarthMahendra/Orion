//
// Created by Samarth Mahendra on 2/27/26.
//

#include "node_runtime.h"

#include <iostream>

namespace orion::distributed {
    // Simple random ID generator (temporary)
    static std::string generate_node_id() {
        static int counter = 0;
        return "node-" + std::to_string(++counter);
    }

    NodeRuntime::NodeRuntime(size_t num_workers,
                         int port,
                         std::string cluster_address)
    : num_workers_(num_workers),
      port_(port),
      cluster_address_(std::move(cluster_address)),
      node_id_(generate_node_id()) {}

    // Start local runtime
    void NodeRuntime::start() {
        if (running_) return;

        std::cout << "[NodeRuntime] Starting node "
                  << node_id_
                  << " on port " << port_ << "\n";

        runtime_ = std::make_unique<orion::Runtime>(num_workers_);

        // 🔜 Later: start RPC server here

        register_with_cluster();   // 👈 NEW

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

    // 🔜 Placeholder registration (Phase 2 control plane hook)
    void NodeRuntime::register_with_cluster() const {
        std::cout << "[NodeRuntime] Registering "
                  << node_id_
                  << " with cluster at "
                  << cluster_address_
                  << "\n";

        // Later this will call:
        // RPC → RegisterNode(node_id, address, resources)
    }

} // namespace orion::distributed