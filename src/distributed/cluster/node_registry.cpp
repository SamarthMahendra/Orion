//
// Created by Samarth Mahendra on 2/27/26.
//

#include "node_registry.h"

#include "node_registry.h"

namespace orion::distributed {

    void NodeRegistry::register_node(const NodeInfo& node) {
        std::lock_guard<std::mutex> lock(mutex_);
        nodes_[node.node_id] = node;
    }

    void NodeRegistry::remove_node(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        nodes_.erase(node_id);
    }

    void NodeRegistry::heartbeat(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (nodes_.count(node_id)) {
            nodes_[node_id].alive = true;
        }
    }

    std::vector<NodeInfo> NodeRegistry::nodes() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<NodeInfo> result;
        result.reserve(nodes_.size());

        for (const auto& [id, node] : nodes_) {
            if (node.alive) {
                result.push_back(node);
            }
        }
        return result;
    }

    std::optional<NodeInfo> NodeRegistry::pick_node() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (nodes_.empty()) return std::nullopt;

        std::vector<NodeInfo> alive_nodes;
        for (const auto& [id, node] : nodes_) {
            if (node.alive) {
                alive_nodes.push_back(node);
            }
        }

        if (alive_nodes.empty()) return std::nullopt;

        NodeInfo chosen = alive_nodes[rr_index_ % alive_nodes.size()];
        rr_index_++;

        return chosen;
    }

} // namespace orion::distributed