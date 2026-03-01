// node_main.cpp
// Starts a single Orion worker node:
//   1. Registers with the head server via gRPC (Milestone 1)
//   2. Runs a NodeService gRPC server so the head can dispatch tasks (Milestone 2)
//
// Usage:  ./node <head_port> <node_port> <node_id>
// Example:./node 50050 6001 node-1
//
// Observable Milestone 2 output:
//   [NodeRuntime] Starting node node-1 on port 6001
//   [NodeRuntime] Registration successful (node=node-1)
//   [Node:node-1] NodeService listening on 0.0.0.0:6001
//   [Node:node-1] Running. Press Ctrl-C to stop.
//   [Node:node-1] ExecuteTask  task=t1  fn=add
//   [Node:node-1] Task complete  fn=add

#include <iostream>
#include <string>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "distributed/node_runtime.h"
#include "distributed/node_service_impl.h"
#include "distributed/functions/function_registry.h"
#include "distributed/functions/builtin_functions.h"

static std::atomic<bool> g_running{true};
static std::unique_ptr<grpc::Server> g_grpc_server;

int main(int argc, char* argv[]) {
    std::string head_host = "localhost";
    int         head_port = 50050;
    int         node_port = 6001;
    std::string node_id   = "node-1";

    if (argc >= 2) head_port = std::stoi(argv[1]);
    if (argc >= 3) node_port = std::stoi(argv[2]);
    if (argc >= 4) node_id   = argv[3];

    std::string cluster_address = head_host + ":" + std::to_string(head_port);
    std::string node_address    = "127.0.0.1:" + std::to_string(node_port);
    std::string listen_address  = "0.0.0.0:" + std::to_string(node_port);

    // ── 1. Build local runtime + register with head ──────────────────────────
    orion::distributed::NodeRuntime node(
        /*num_workers=*/2,
        node_port,
        cluster_address,
        node_id,
        node_address
    );
    node.start();   // registers with head internally

    // ── 2. Build function registry with builtins ─────────────────────────────
    orion::distributed::FunctionRegistry fn_reg;
    orion::distributed::register_builtin_functions(fn_reg);

    // ── 3. Start NodeService gRPC server ─────────────────────────────────────
    orion::distributed::NodeServiceImpl node_service(node, fn_reg);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(listen_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&node_service);

    g_grpc_server = builder.BuildAndStart();
    if (!g_grpc_server) {
        std::cerr << "[Node:" << node_id << "] Failed to start NodeService on "
                  << listen_address << "\n";
        node.stop();
        return 1;
    }
    std::cout << "[Node:" << node_id << "] NodeService listening on "
              << listen_address << "\n" << std::flush;

    // ── 4. Run until Ctrl-C ───────────────────────────────────────────────────
    std::signal(SIGINT, [](int) {
        g_running = false;
        if (g_grpc_server) g_grpc_server->Shutdown();
    });

    std::cout << "[Node:" << node_id << "] Running. Press Ctrl-C to stop.\n"
              << std::flush;

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    g_grpc_server->Shutdown();
    node.stop();
    return 0;
}
