# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -g

# Boost libraries
BOOST_LIBS = -lboost_system -lboost_thread -lWs2_32 -lwsock32

EXEC = expose proxy_server echo_server

# Default target
all: $(EXEC)


proxy_server: proxy_server.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BOOST_LIBS)

# Link the executable
expose: expose.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BOOST_LIBS)

echo_server: echo_server.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BOOST_LIBS)


# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets
.PHONY: all clean