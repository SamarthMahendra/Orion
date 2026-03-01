// node_main.cpp
// Starts a single Orion node and registers it with the head server via gRPC.
// Usage:  ./node <head_port> <node_port> <node_id>
// Example:./node 50050 6001 node-1
//
// Observable milestone output:
//   [NodeRuntime] Starting node node-1 on port 6001
//   [NodeRuntime] Registering node-1 with cluster at localhost:50050
//   [NodeRuntime] Registration successful

#include <iostream>
#include <string>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

#include "distributed/node_runtime.h"

static std::atomic<bool> g_running{true};

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

    orion::distributed::NodeRuntime node(
        /*num_workers=*/2,
        node_port,
        cluster_address,
        node_id,
        node_address
    );

    node.start();   // registers with cluster internally

    // Handle Ctrl-C gracefully
    std::signal(SIGINT, [](int) { g_running = false; });

    std::cout << "[Node:" << node_id << "] Running. Press Ctrl-C to stop.\n" << std::flush;

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    node.stop();
    return 0;
}
