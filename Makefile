# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -g

# Detect OS (Linux vs Windows MinGW / MSYS)
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    BOOST_LIBS = -lboost_system -lboost_thread
endif

ifeq ($(UNAME_S),Darwin)
    BOOST_LIBS = -lboost_system -lboost_thread
endif

# 假設 Windows 環境是 MINGW/MSYS (uname 會輸出 MINGW32_NT 或 MINGW64_NT)
ifneq (,$(findstring MINGW,$(UNAME_S)))
    BOOST_LIBS = -lboost_system -lboost_thread -lWs2_32 -lwsock32
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

