# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -g

# Detect OS (Linux vs Windows MinGW / MSYS)
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    $(info 🐧 Linux: Using core Boost libraries only)
    BOOST_LIBS = -lboost_system -lboost_thread
    EXE_EXT =

else ifeq ($(UNAME_S),Darwin)
    $(info 🍎 macOS: Using core Boost libraries only)
    BOOST_LIBS = -lboost_system -lboost_thread
    EXE_EXT =

else
    $(info 🪟 Windows: Including Winsock libraries (-lWs2_32 -lmswsock))
    BOOST_LIBS = -lboost_system -lboost_thread -lWs2_32 -lmswsock
    EXE_EXT = .exe
endif

EXEC = expose$(EXE_EXT) proxy_server$(EXE_EXT) echo_server$(EXE_EXT)

# Default target
all: $(EXEC) install-hooks

# Generic rule: build <target>$(EXE_EXT) from <target>.o
%$(EXE_EXT): %.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BOOST_LIBS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Install git pre-commit hook
install-hooks:
	@echo "🔧 Installing pre-commit hook..."
	@sh -c 'cp .ci/pre-commit.sh .git/hooks/pre-commit && chmod +x .git/hooks/pre-commit'
	@echo "✅ pre-commit hook installed"

# Clean up build files
clean:
	rm -f *.o $(EXEC)

# Phony targets
.PHONY: all clean install-hooks
