# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -g

# Detect OS (Linux vs Windows MinGW / MSYS)
# NOTE: The "process_begin: CreateProcess(NULL, uname -s, ...) failed" error
# is expected if running in PowerShell/Cmd, but the 'else' block below
# ensures the correct Windows libraries are used anyway.
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    # Use $(info ...) to print messages during parsing
    $(info 🐧 Linux: Using core Boost libraries only)
    BOOST_LIBS = -lboost_system -lboost_thread

else ifeq ($(UNAME_S),Darwin)
    $(info 🍎 macOS: Using core Boost libraries only)
    BOOST_LIBS = -lboost_system -lboost_thread

else
    # 🪟 Windows (Catches MINGW, CYGWIN, and other Windows shells)
    $(info 🪟 Windows: Including Winsock libraries (-lWs2_32 -lmswsock))
    BOOST_LIBS = -lboost_system -lboost_thread -lWs2_32 -lmswsock

endif

EXEC = expose proxy_server echo_server

# Default target
all: $(EXEC)

proxy_server: proxy_server.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BOOST_LIBS)

expose: expose.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BOOST_LIBS)

echo_server: echo_server.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BOOST_LIBS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f *.o $(EXEC)

# Phony targets
.PHONY: all clean

