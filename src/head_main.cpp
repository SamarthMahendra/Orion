// head_main.cpp — Orion Cluster Head Server
// Implements the gRPC ClusterHead service.
//
// Usage:  ./head [port]   (default: 50050)
//
// Milestone 1 observable output:
//   [Head] Listening on 0.0.0.0:50050
//   [Head] RegisterNode  node=node-1  addr=127.0.0.1:6001
//   [Head] RegisterNode  node=node-2  addr=127.0.0.1:6002

#include <iostream>
#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "distributed/generated/orion.grpc.pb.h"
#include "distributed/cluster/node_registry.h"
#include "distributed/cluster/cluster_scheduler.h"
#include "distributed/rpc/node_client.h"

// ── Noop NodeClient ──────────────────────────────────────────────────────────
// The head doesn't dispatch tasks to nodes yet (Milestone 2).
struct NoopNodeClient : public orion::distributed::NodeClient {
    orion::ObjectRef submit_task(const std::string& node_id, orion::Task task) override {
        std::cout << "[NodeClient:NOOP] would ExecuteTask(" << task.id
                  << ") on " << node_id << "\n" << std::flush;
        return orion::ObjectRef{task.id};
    }
};

// ── gRPC ClusterHead service implementation ──────────────────────────────────
class HeadServiceImpl final : public orion::ClusterHead::Service {
public:
    HeadServiceImpl(orion::distributed::NodeRegistry& registry,
                    orion::distributed::ClusterScheduler& scheduler)
        : registry_(registry), scheduler_(scheduler) {}

    // ── Milestone 1 ─────────────────────────────────────────────────────────
    grpc::Status RegisterNode(grpc::ServerContext*,
                              const orion::RegisterNodeRequest* req,
                              orion::RegisterNodeReply* reply) override {
        std::cout << "[Head] RegisterNode  node=" << req->node_id()
                  << "  addr=" << req->address() << "\n" << std::flush;
        registry_.register_node({req->node_id(), req->address(), 0, true});
        reply->set_success(true);
        return grpc::Status::OK;
    }

    // ── Milestone 2 (stubbed) ────────────────────────────────────────────────
    grpc::Status SubmitTask(grpc::ServerContext*,
                            const orion::TaskRequest* req,
                            orion::TaskReply*) override {
        std::cout << "[Head] SubmitTask  task=" << req->task_id()
                  << "  (TODO)\n" << std::flush;
        return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "not yet implemented");
    }

    // ── Milestone 3 (stubbed) ────────────────────────────────────────────────
    grpc::Status ReportObjectCreated(grpc::ServerContext*,
                                     const orion::ObjectReport* req,
                                     orion::Empty*) override {
        std::cout << "[Head] ReportObjectCreated  object=" << req->object_id()
                  << "  node=" << req->node_id() << "  (TODO)\n" << std::flush;
        return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "not yet implemented");
    }

    grpc::Status GetObjectLocation(grpc::ServerContext*,
                                   const orion::ObjectLocationRequest* req,
                                   orion::ObjectLocationReply*) override {
        std::cout << "[Head] GetObjectLocation  object=" << req->object_id()
                  << "  (TODO)\n" << std::flush;
        return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "not yet implemented");
    }

private:
    orion::distributed::NodeRegistry&     registry_;
    orion::distributed::ClusterScheduler& scheduler_;
};

// ── main ─────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    std::string port           = (argc > 1) ? argv[1] : "50050";
    std::string server_address = "0.0.0.0:" + port;

    orion::distributed::NodeRegistry     registry;
    NoopNodeClient                        noop_client;
    orion::distributed::ClusterScheduler scheduler(registry, noop_client);

    HeadServiceImpl service(registry, scheduler);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    auto server = builder.BuildAndStart();
    if (!server) {
        std::cerr << "[Head] Failed to start on " << server_address << "\n";
        return 1;
    }
    std::cout << "[Head] Listening on " << server_address << "\n" << std::flush;
    server->Wait();
    return 0;
}
