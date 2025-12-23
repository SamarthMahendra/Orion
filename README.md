# Orion

A lightweight, multi-threaded task execution framework in C++ with dependency-aware scheduling for parallel computing.

## Overview

Orion is a task execution system designed to provide a simple abstraction for executing computational tasks with dependency management. The framework is built around three core components: **Tasks**, **Workers**, and a **Scheduler**.

## Architecture

### Task (`task.h`)

The fundamental unit of execution in Orion. A Task represents work that can be executed by a worker, with support for dependencies on other tasks.

```cpp
struct Task {
    std::string id;                           // Unique identifier
    std::vector<ObjectRef> deps;              // Dependencies on other tasks
    std::function<std::any(std::vector<std::any>)> work;  // Executable work that receives dependency values
};
```

**Key Features:**
- Unique task identifiers for tracking
- Dependency management via ObjectRef vector
- Work functions that can accept values from dependent tasks
- Type-safe result storage using `std::any`

### Worker (`worker.h`, `worker.cpp`)

An execution unit responsible for running tasks in a dedicated background thread. Workers maintain a thread-safe queue and continuously execute tasks, storing results in an object store.

**Key Features:**
- Runs in dedicated background thread for true parallelism
- Thread-safe task queue with mutex and condition variables
- Non-blocking task submission
- Automatic task execution loop
- Integration with object store for result storage

**API:**
- `ObjectRef submit(Task task)` - Enqueue a task for execution, returns reference to future result
- `void start()` - Start the worker's background thread
- `void stop()` - Stop the worker's background thread

### Scheduler (`scheduler.h`, `scheduler.cpp`)

A dataflow scheduler that manages task dependencies and dispatches runnable tasks to workers.

**Key Features:**
- Automatic dependency tracking and resolution
- Round-robin task distribution across multiple workers
- Maintains pending and ready task queues
- Callback-driven scheduling on object creation

**API:**
- `void submit(Task task)` - Submit a task to the scheduler
- `void on_object_created(const ObjectId& id)` - Notification when a new object is created
- `void schedule()` - Attempt to schedule ready tasks to workers

### Object Store (`object_store.h`, `object_store.cpp`)

A thread-safe in-memory key-value store for storing task results and intermediate data.

**Key Features:**
- Thread-safe storage using mutex and condition variables
- Type-safe storage using `std::any`
- Blocking and non-blocking get operations
- Callback support for notifying scheduler of new objects

**API:**
- `void put(const ObjectId& id, std::any value)` - Store a value
- `std::optional<std::any> get(const ObjectId& id)` - Non-blocking retrieve
- `std::any get_blocking(const ObjectId& id)` - Blocking retrieve that waits until value exists
- `void set_on_put_callback(OnPutCallback callback)` - Register callback for object creation

### Object Reference (`object_ref.h`)

A lightweight reference to objects stored in the object store.

```cpp
struct ObjectRef {
    ObjectId id;  // Unique identifier for the stored object
};
```

## Usage

```cpp
// Create object store and workers
orion::ObjectStore store;
orion::Worker w1(store), w2(store);
orion::Scheduler scheduler({&w1, &w2}, store);

// Start workers
w1.start();
w2.start();

// Submit tasks
orion::Task task{"T1", {}, [](std::vector<std::any>) { return 42; }};
scheduler.submit(task);
scheduler.schedule();

// Wait for result
auto result = store.get_blocking("T1");

// Clean up
w1.stop();
w2.stop();
```

See `main.cpp` for a complete parallel execution example.

## Building

This project uses C++17 or later. To build:

```bash
g++ -std=c++17 main.cpp worker.cpp scheduler.cpp object_store.cpp -o main -pthread
./main
```

Or with clang:

```bash
clang++ -std=c++17 main.cpp worker.cpp scheduler.cpp object_store.cpp -o main -pthread
./main
```

## Design Principles

1. **Separation of Concerns**: Workers execute tasks, scheduler manages dependencies
2. **Thread Safety**: All operations protected by mutexes and condition variables
3. **Dataflow Model**: Tasks execute when dependencies are satisfied
4. **Simplicity**: Minimal abstraction with clear responsibilities

## Current Status

The project is in active development. Implemented features:
- Multi-threaded workers with background execution loops
- Dependency-aware task scheduling
- Task dependency tracking and resolution
- Thread-safe object store with blocking/non-blocking get
- Round-robin task distribution across workers
- Callback-driven dependency resolution

## Future Roadmap

- Advanced scheduling policies (priority-based, work-stealing)
- Distributed task execution across machines
- Task failure handling and retry mechanisms
- Resource hints and constraints
- Performance monitoring and metrics

## License

MIT License

## Author

Samarth Mahendra