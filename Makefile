CXX := clang++
CXXFLAGS := -std=c++23 -Wall -Wextra -O2 -pthread

TARGET := main
SRCS := src/main.cpp src/core/worker.cpp src/core/object_store.cpp src/core/scheduler.cpp src/local/runtime.cpp src/distributed/node_runtime.cpp
OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
