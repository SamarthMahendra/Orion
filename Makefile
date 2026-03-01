CXX := clang++
CXXFLAGS := -std=c++23 -Wall -Wextra -O2 -pthread

export PKG_CONFIG_PATH := /opt/homebrew/opt/grpc/lib/pkgconfig:/opt/homebrew/opt/protobuf/lib/pkgconfig:/opt/homebrew/opt/abseil/lib/pkgconfig

SRC := src
GEN_DIR := $(SRC)/distributed/generated

GRPC_INC := $(shell pkg-config --cflags grpc++ protobuf) -I$(SRC)
GRPC_LIB := $(shell pkg-config --libs grpc++ protobuf)

GEN_SRCS := \
    $(GEN_DIR)/orion.pb.cc \
    $(GEN_DIR)/orion.grpc.pb.cc

GEN_OBJS := $(GEN_SRCS:.cc=.o)

CORE_SRCS := \
    $(SRC)/core/worker.cpp \
    $(SRC)/core/object_store.cpp \
    $(SRC)/core/scheduler.cpp \
    $(SRC)/local/runtime.cpp

CLUSTER_SRCS := \
    $(SRC)/distributed/cluster/cluster_scheduler.cpp \
    $(SRC)/distributed/cluster/node_registry.cpp

NODE_RT_SRC := $(SRC)/distributed/node_runtime.cpp

MAIN_SRCS := $(SRC)/main.cpp $(CORE_SRCS) $(CLUSTER_SRCS) $(NODE_RT_SRC)
MAIN_OBJS := $(MAIN_SRCS:.cpp=.o)

HEAD_SRCS := $(SRC)/head_main.cpp $(CORE_SRCS) $(CLUSTER_SRCS)
HEAD_OBJS := $(HEAD_SRCS:.cpp=.o)

NODE_SRCS := $(SRC)/node_main.cpp $(CORE_SRCS) $(NODE_RT_SRC)
NODE_OBJS := $(NODE_SRCS:.cpp=.o)

main: $(MAIN_OBJS) $(GEN_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(GRPC_LIB) -o main

head: $(HEAD_OBJS) $(GEN_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(GRPC_LIB) -o head

node: $(NODE_OBJS) $(GEN_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(GRPC_LIB) -o node

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(GRPC_INC) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(GRPC_INC) -c $< -o $@

clean:
	rm -f $(SRC)/**/*.o main head node 2>/dev/null || true

.PHONY: main head node clean