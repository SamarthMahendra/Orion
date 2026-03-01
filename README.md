# Orion

A multi-threaded, dependency-aware task execution framework in C++23 — designed to scale from a single-machine runtime to a distributed cluster of worker nodes.

## Overview

Orion models computation as a **dataflow graph**: tasks declare their inputs as `ObjectRef` dependencies, and the scheduler dispatches them to workers only when all dependencies are satisfied. The framework is layered into three tiers:

| Tier | What it does |
|---|---|
| **Core** | Tasks, workers, scheduler, object store — the engine |
| **Local** | `Runtime` — a clean façade over the core for single-process use |
| **Distributed** | `NodeRuntime`, `ClusterScheduler`, `NodeRegistry`, `NodeClient` — multi-node execution |

---

## Architecture

### Core Layer (`src/core/`)

<img width="1512" height="1314" alt="image" src="https://github.com/user-attachments/assets/cbbf2a5f-81b9-4fb3-bd69-4b6181a546b5" />

https://excalidraw.com/#json=vnPHTMiI-ZkdczEkcpeKh,bthOxQcgldqqnRsoox0SSg


## Project Structure

```
Orion/
├── src/
│   ├── main.cpp                          # Entry point / integration demo
│   ├── core/
│   │   ├── task.h                        # Task struct
│   │   ├── object_ref.h                  # ObjectRef / ObjectId
│   │   ├── object_store.{h,cpp}          # Thread-safe result store
│   │   ├── worker.{h,cpp}                # Background-thread executor
│   │   └── scheduler.{h,cpp}             # Local dataflow scheduler
│   ├── local/
│   │   └── runtime.{h,cpp}               # Single-process Runtime façade
│   └── distributed/
│       ├── node_runtime.{h,cpp}          # Per-node runtime wrapper
│       ├── cluster/
│       │   ├── node_registry.{h,cpp}     # Cluster membership + node selection
│       │   └── cluster_scheduler.{h,cpp} # Cross-node dataflow scheduler
│       ├── rpc/
│       │   ├── node_client.h             # Abstract RPC interface
│       │   ├── inprocess_node_client.h   # In-process stub (testing)
│       │   └── grpc_node_client.h        # Real gRPC transport implementation
│       └── proto/
│           └── orion.proto               # cluster communication definitions
├── head_main.cpp                         # Cluster Head server entry point
├── node_main.cpp                         # Worker Node entry point
├── submit_test.cpp                       # gRPC task submission test
├── Makefile
└── LICENSE
```

---

#### Task (`task.h`)

The fundamental unit of work.

```cpp
struct Task {
    std::string id;                           // unique identifier / output key
    std::vector<ObjectRef> deps;              // IDs of required input objects
    std::function<std::any(const std::vector<std::any>&)> work;
};
```

#### ObjectRef / ObjectId (`object_ref.h`)

A lightweight handle to a future or present result stored in the `ObjectStore`.

```cpp
struct ObjectRef { ObjectId id; };
```

#### ObjectStore (`object_store.h/cpp`)

Thread-safe, in-memory key-value store for task results.

| Method | Behaviour |
|---|---|
| `put(id, value)` | Store a result; triggers the registered callback |
| `get(id)` | Non-blocking; returns `std::nullopt` if absent |
| `get_blocking(id)` | Blocks until the value is available |
| `set_on_put_callback(fn)` | Notify scheduler when a new object lands |

#### Worker (`worker.h/cpp`)

Owns a single background thread. Dequeues tasks, resolves dependency values from the object store, and invokes `task.work`. Supports **work-stealing friendly** queueing via mutex + condition variable.

| Method | Behaviour |
|---|---|
| `submit(task)` → `ObjectRef` | Enqueue a task; returns its output ref |
| `start()` / `stop()` | Lifecycle control |

#### Scheduler (`scheduler.h/cpp`)

Dataflow scheduler that sits between callers and workers.

- Tracks all submitted tasks in a `pending` map
- When `on_object_created` fires, re-evaluates readiness of waiting tasks
- Dispatches ready tasks to workers via **round-robin**

---

### Local Layer (`src/local/`)

#### Runtime (`runtime.h/cpp`)

A single-process, batteries-included entry point. Owns the object store, N workers, and the scheduler—hiding all wiring from the caller.

```cpp
orion::Runtime rt(4);           // 4 worker threads

orion::Task t{"square", {}, [](const std::vector<std::any>&) -> std::any {
    return 6 * 6;
}};

auto ref = rt.submit(t);
rt.wait(ref);

int result = std::any_cast<int>(rt.get(ref)); // 36
rt.shutdown();
```

---

### Distributed Layer (`src/distributed/`)

#### NodeRuntime (`node_runtime.h/cpp`)

Represents a single physical (or logical) node in the cluster. Wraps a `Local::Runtime` and will eventually host an RPC server.

- Auto-generates a unique `node_id` on construction
- Calls `register_with_cluster()` on `start()` (currently logs; RPC hook is stubbed for Phase 2)
- Configurable worker count and port number

#### NodeRegistry (`cluster/node_registry.h/cpp`)

Maintains the live set of nodes known to the cluster.

| Method | Behaviour |
|---|---|
| `register_node(info)` | Add or update a node |
| `remove_node(id)` | Mark a node dead |
| `heartbeat(id)` | Update liveness (future: TTL-based eviction) |
| `pick_node()` | Round-robin node selection |

`NodeInfo` carries `node_id`, `address` (`host:port`), `available_workers`, and an `alive` flag.

#### ClusterScheduler (`cluster/cluster_scheduler.h/cpp`)

Cluster-wide counterpart to the local `Scheduler`.

- Accepts tasks via `submit(task)`
- Gates dispatch on dep readiness (checks `object_locations_` map)
- Picks a target node from `NodeRegistry` and fires `NodeClient::submit_task`
- Records expected object location optimistically at dispatch time (v0.2 assumption; node-reported confirmations are planned)

```
ClusterScheduler::submit(task)
    └── schedule()
            ├── deps_ready_?  [check object_locations_]
            ├── registry_.pick_node()
            ├── client_.submit_task(node_id, task)
            └── on_object_created(task.id, node_id)
```

#### NodeClient (`rpc/node_client.h`)

Abstract interface for sending tasks to a node.

```cpp
class NodeClient {
public:
    virtual ObjectRef submit_task(const std::string& node_id, Task task) = 0;
};
```

#### InProcessNodeClient (`rpc/inprocess_node_client.h`)

Concrete `NodeClient` for testing and single-binary cluster simulation. Holds raw pointers to `NodeRuntime` instances and routes calls directly — no network involved.

---

## Usage Examples

### Multi-node cluster (gRPC)

Start the cluster head server:

```bash
./head 50050
```

Start one or more worker nodes in separate terminals:

```bash
./node 50050 6001 node-1
./node 50050 6002 node-2
```

Submit test tasks to the cluster:

```bash
./submit_test 50050
```

### Single-process Debug (Local Runtime)

```cpp

    NodeRuntime n1(2, 5001);
    NodeRuntime n2(2, 5002);

    n1.start();
    n2.start();

    NodeRegistry registry;
    registry.register_node({"node-1", "localhost:5001", 2, true});
    registry.register_node({"node-2", "localhost:5002", 2, true});

    InProcessNodeClient client;
    client.add_node("node-1", &n1);
    client.add_node("node-2", &n2);

    ClusterScheduler cluster(registry, client);

    // Task A
    orion::Task t1{
        "A",
        {},
        [](const std::vector<std::any>&) -> std::any { return 10; }
    };

    // Task B depends on A
    orion::Task t2{
        "B",
        {orion::ObjectRef{"A"}},
        [](std::vector<std::any> args) -> std::any {
            int a = std::any_cast<int>(args[0]);
            return a + 32;
        }
    };

    cluster.submit(t1);
    cluster.submit(t2);

    // In v0.2 we assumed locations at dispatch time;
    // real completion tracking comes next (heartbeats/object reports).
    // For now you can just sleep or block at node-local store if you expose it.

    std::cout << "Cluster scheduled tasks.\n";

    n1.stop();
    n2.stop();
```

### Multi-node cluster (in-process simulation)

```cpp
using namespace orion::distributed;

NodeRuntime n1(2, 5001), n2(2, 5002);
n1.start(); n2.start();

NodeRegistry registry;
registry.register_node({"node-1", "localhost:5001", 2, true});
registry.register_node({"node-2", "localhost:5002", 2, true});

InProcessNodeClient client;
client.add_node("node-1", &n1);
client.add_node("node-2", &n2);

ClusterScheduler cluster(registry, client);

// Task A: no deps
orion::Task t1{"A", {}, [](const std::vector<std::any>&) -> std::any { return 10; }};

// Task B: depends on A
orion::Task t2{"B", {orion::ObjectRef{"A"}},
    [](std::vector<std::any> args) -> std::any {
        return std::any_cast<int>(args[0]) + 32; // 42
    }};

cluster.submit(t1);
cluster.submit(t2);

n1.stop(); n2.stop();
```

---


## Building

Requires **C++23** and a POSIX-compatible system (pthreads).

```bash
# Build with Make (recommended)
make

# Clean
make clean
```

The Makefile uses `clang++` with `-std=c++23 -O2 -pthread`. To use GCC:

```bash
CXX=g++ make
```

---

## Design Principles

1. **Dataflow semantics** — tasks run when their inputs exist, not when the caller says so
2. **Layered design** — core engine is network-agnostic; distribution is opt-in
3. **Thread safety throughout** — mutexes + condition variables at every shared boundary
4. **Pluggable transport** — `NodeClient` abstraction decouples scheduling from RPC implementation
5. **Test-friendly** — `InProcessNodeClient` lets you run a full cluster in a single binary

---

## Current Status

### Implemented
- [x] Core task execution engine (workers, scheduler, object store)
- [x] Local `Runtime` façade
- [x] `NodeRuntime` (per-node wrapper with lifecycle management)
- [x] `NodeRegistry` (cluster membership, round-robin selection, heartbeat stub)
- [x] `ClusterScheduler` (cross-node dependency tracking and dispatch)
- [x] `NodeClient` abstraction + `InProcessNodeClient` for in-process testing
- [x] Multi-node dependency-chaining demo in `main.cpp`
- [x] **Real RPC transport using gRPC** (`head`, `node`, `submit_test` executables)

### In Progress / Planned

#### Phase 2 — Real Transport & Fault Tolerance
- [x] Real RPC transport (gRPC or custom TCP) replacing `InProcessNodeClient`
- [ ] Node-reported object location confirmations (replacing optimistic v0.2 assumption)
- [ ] Heartbeat-based node liveness and dead-node eviction
- [ ] Task failure handling and retry with configurable policies
- [ ] Work-stealing across nodes

#### Phase 3 — Ad-hoc Distributed Data Computation
- [ ] Cross-process object serialization (replace `std::any` with a wire format)
- [ ] Cross-node object transfer — automatic fetch when an input object lives on a remote node
- [ ] Distributed object store (shared-memory + TCP pull, similar to Ray Plasma)
- [ ] Streaming / chunked object support for large datasets
- [ ] Map / reduce primitives built on top of the task graph

#### Phase 4 — Dynamic Task Graphs
- [ ] Tasks can submit sub-tasks at runtime (nested parallelism)
- [ ] Support for recursive and speculative task patterns
- [ ] `ObjectRef` as a first-class future passable between tasks at runtime
- [ ] Group / barrier synchronisation primitives

#### Phase 5 — GPU & Heterogeneous Scheduling
- [ ] Resource annotations on `Task` (CPU cores, GPU count, memory)
- [ ] GPU-aware node selection in `ClusterScheduler` / `NodeRegistry`
- [ ] CUDA stream integration for GPU task execution
- [ ] Mixed CPU + GPU pipeline scheduling

#### Phase 6 — Global Control Store (GCS)
- [ ] Centralized GCS process for cluster-wide state (node registry, object table)
- [ ] Automatic node discovery and registration via GCS on `NodeRuntime::start()`
- [ ] Fault-tolerant GCS with persistent backing store
- [ ] Pub/sub object-ready notifications replacing polling

#### Phase 7 — Dashboard & Observability
- [ ] REST API exposing cluster state (nodes, tasks, object locations)
- [ ] Web dashboard: live task graph visualisation, node health, throughput metrics
- [ ] Distributed tracing (task lineage from submission to completion)
- [ ] Prometheus-compatible metrics endpoint

---

## License

MIT License

## Author

Samarth Mahendra
