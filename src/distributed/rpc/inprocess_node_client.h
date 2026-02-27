//
// Created by Samarth Mahendra on 2/27/26.
//

#ifndef INPROCESS_NODE_CLIENT_H
#define INPROCESS_NODE_CLIENT_H


#pragma once

#include <unordered_map>
#include <string>
#include <stdexcept>

#include "node_client.h"
#include "../node_runtime.h"

namespace orion::distributed {

    class InProcessNodeClient : public NodeClient {
    public:
        void add_node(const std::string& node_id, NodeRuntime* node) {
            nodes_[node_id] = node;
        }

        orion::ObjectRef submit_task(const std::string& node_id, orion::Task task) override {
            auto it = nodes_.find(node_id);
            if (it == nodes_.end() || it->second == nullptr) {
                throw std::runtime_error("Unknown node_id: " + node_id);
            }
            return it->second->local_runtime().submit(std::move(task));
        }

    private:
        std::unordered_map<std::string, NodeRuntime*> nodes_;
    };

} // namespace orion::distributed

#endif //INPROCESS_NODE_CLIENT_H
