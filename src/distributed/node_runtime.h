//
// Created by Samarth Mahendra on 2/27/26.
//

#ifndef NODE_RUNTIME_H
#define NODE_RUNTIME_H


#pragma once

#include <cstddef>
#include <memory>

#include "../local/runtime.h"

namespace orion::distributed {

    // Represents a single Orion node (one machine)
    // Owns a local runtime and will later host RPC services
    class NodeRuntime {
    public:
        // num_workers = worker threads on this node
        // port = RPC port (used later)
        NodeRuntime(size_t num_workers,
                          int port,
                          std::string cluster_address="");


        // Start node (workers + RPC server later)
        void start();

        // Graceful shutdown
        void stop();

        // this will let cluster know hi i am here with x cores
        void register_with_cluster() const;   // 👈 NEW

        // Access local runtime (useful for testing)
        orion::Runtime& local_runtime();

    private:
        std::unique_ptr<orion::Runtime> runtime_;
        size_t num_workers_;
        int port_;

        // to know what cluster does this node belong to
        std::string cluster_address_;

        // for cluster to know what node
        std::string node_id_;

        bool running_ = false;
    };

} // namespace orion::distributed

#endif //NODE_RUNTIME_H
