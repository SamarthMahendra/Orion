CXX := clang++
CXXFLAGS := -std=c++23 -Wall -Wextra -O2 -pthread

TARGET := main
SRCS := main.cpp worker.cpp object_store.cpp scheduler.cpp
OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
