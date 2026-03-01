//
// GrpcNodeClient — head-side NodeClient implementation that calls
// NodeService::ExecuteTask on real worker nodes via gRPC.
//

#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <iostream>

#include <grpcpp/grpcpp.h>
#include "../generated/orion.grpc.pb.h"

#include "node_client.h"
#include "../cluster/node_registry.h"

namespace orion::distributed {

class GrpcNodeClient : public NodeClient {
public:
    explicit GrpcNodeClient(NodeRegistry& registry)
        : registry_(registry) {}

    // Convert orion::Task → TaskRequest proto and call NodeService::ExecuteTask.
    // The task MUST have function_name set; dep_ids are passed as proto repeated strings.
    orion::ObjectRef submit_task(const std::string& node_id,
                                 orion::Task task) override
    {
        auto* stub = get_or_create_stub(node_id);
        if (!stub) {
            std::cerr << "[GrpcNodeClient] No stub for node=" << node_id << "\n";
            return orion::ObjectRef{task.id};
        }

        // Build proto request
        ::orion::TaskRequest req;
        req.set_task_id(task.id);
        req.set_function_name(task.function_name);
        for (const auto& dep : task.deps) {
            req.add_dep_ids(dep.id);
        }
        // Forward serialized literal args bytes to the node
        for (const auto& bytes : task.args) {
            req.add_args(bytes);
        }

        ::orion::TaskReply reply;
        grpc::ClientContext ctx;

        grpc::Status status = stub->ExecuteTask(&ctx, req, &reply);

        if (status.ok() && reply.accepted()) {
            std::cout << "[GrpcNodeClient] ExecuteTask(" << task.id
                      << ") accepted by " << node_id << "\n" << std::flush;
        } else {
            std::cerr << "[GrpcNodeClient] ExecuteTask FAILED for task="
                      << task.id << ": " << status.error_message() << "\n";
        }

        return orion::ObjectRef{task.id};
    }

private:
    // Returns raw pointer to stub (owned by stubs_ map).
    // Creates a new stub if this node_id hasn't been seen yet.
    orion::NodeService::Stub* get_or_create_stub(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(mu_);

        auto it = stubs_.find(node_id);
        if (it != stubs_.end()) {
            return it->second.get();
        }

        // Look up the node's address from the registry
        auto nodes = registry_.nodes();
        std::string address;
        for (const auto& n : nodes) {
            if (n.node_id == node_id) {
                address = n.address;
                break;
            }
        }

        if (address.empty()) {
            std::cerr << "[GrpcNodeClient] Unknown node_id=" << node_id << "\n";
            return nullptr;
        }

        auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        auto stub = orion::NodeService::NewStub(channel);
        auto* ptr = stub.get();
        stubs_[node_id] = std::move(stub);
        return ptr;
    }

    NodeRegistry& registry_;
    std::unordered_map<std::string, std::unique_ptr<orion::NodeService::Stub>> stubs_;
    std::mutex mu_;
};

} // namespace orion::distributed
