# Contributing Guide

Thank you for your interest in contributing to our Reverse Proxy project! We welcome contributions of all kinds, including bug reports, feature suggestions, documentation improvements, and code submissions.


#### Table of Contents

- [Issues](#issues)
  - [Reporting Bugs](#reporting-bugs)
  - [Suggesting Features](#suggesting-features)
- [Development Environment](#development-environment)
  - [Prerequisites](#prerequisites)
  - [Setting Up Development Environment](#setting-up-development-environment)
  - [Building the Project](#building-the-project)
- [Code Style](#code-style)
  - [Refactor / Autoformat with astyle](#refactor--autoformat-with-astyle)
  - [Static Analysis](#static-analysis)
  - [Coding Standards](#coding-standards)
  - [Example Code Style](#example-code-style)
- [Project Structure](#project-structure)
- [Pull Request](#pull-request)
  - [Commit Message Style](#commit-message-style)
  - [Documentation](#documentation)
  - [Review Process](#review-process)


---

## Issues

### Reporting Bugs

If you find a bug, please create an issue with the following information:

1. **Clear description** of the problem
2. **Steps to reproduce** the issue
3. **Expected behavior**
4. **Actual behavior**
5. **Environment details** (OS, compiler version, etc.)
6. **Relevant logs or screenshots**

### Suggesting Features

We welcome feature suggestions! Please:

* Check if there's already a similar feature request
* Clearly describe the use case and value of the feature
* Provide implementation suggestions if possible

---

## Development Environment

### Prerequisites

* C++17 compatible compiler (GCC 7+, or MSVC 2019+)
* CMake 3.12+
* Boost libraries (System, Thread)

### Setting Up Development Environment

#### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt update
sudo apt install -y g++ make cmake libboost-system-dev libboost-thread-dev

# Optional: Development tools
sudo apt install -y cppcheck astyle gdb
```

#### Windows Installation Steps

* Install **Visual Studio 2019 or later** with **C++ support**.
* Install **CMake**, **cppcheck**, and **astyle**. It’s recommended to use [**Scoop**](https://scoop.sh/) for installation.
* Install **Boost libraries**.

### Building the Project

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .

# For Release builds
cmake --build . --config Release
```

---

## Code Style

We use the following tools to maintain code quality:

### Refactor / Autoformat with astyle

This project provides an `.astylerc` configuration (Google style variant) for the AStyle formatter. To automatically refactor C/C++ code using the project's style rules, run:

```bash
astyle --options=.astylerc $(find src include -type f -regex '.*\.\(c\|cpp\|h\|hpp\)$')
```

### Static Analysis

```bash
run_cppcheck() {
    local target_file=$1
    echo "Checking $target_file..."
    
    cppcheck \
        --enable=all \
        --inconclusive \
        --std=c++17 \
        --library=std.cfg \
        --library=posix.cfg \
        -I include \
        --suppress=missingIncludeSystem \
        --quiet \
        --error-exitcode=1 \
        "$target_file" include/*.hpp
}

run_cppcheck "src/echo_server.cpp"
run_cppcheck "src/proxy_server.cpp"
run_cppcheck "src/expose.cpp"
```

### Coding Standards

* Use **snake_case** for variables and functions
* Use **PascalCase** for classes and structs
* Use **UPPER_CASE** for constants
* Include header guards in all header files
* Prefer `const` and `constexpr` where applicable
* Use smart pointers instead of raw pointers

### Example Code Style

```cpp
#ifndef PROJECT_CLASS_NAME_H
#define PROJECT_CLASS_NAME_H

#include <boost/asio.hpp>
#include <memory>
#include <string>

class ExampleClass {
public:
    explicit ExampleClass(std::string name);
    
    void perform_action();
    const std::string& get_name() const { return name_; }

private:
    void helper_method_();
    
    std::string name_;
    boost::asio::io_context io_context_;
};

#endif // PROJECT_CLASS_NAME_H
```

---

## Project Structure

```
reverse-proxy/
├── .githooks/      # Git hooks
├── src/            # Source code
├── include/        # Public headers
├── docs/           # Documentation
└── cmake/          # CMake modules
```

---

## Pull Request

Before opening a PR, make sure your commit messages follow our conventions and general best practices.

### Commit Message Style

We follow the **Conventional Commits** format for clarity and consistency.
A commit message should start with a short, descriptive title, followed by an optional body that explains the change in more detail.

#### Format

```
<prefix>: <short summary>

[optional body]
```

#### Allowed Prefixes

| Prefix    | Usage                                                 |
| :-------- | :---------------------------------------------------- |
| `feat:`   | New user-facing features or changes                   |
| `fix:`    | Bug fixes                                             |
| `refact:` | Code refactoring or cleanup without changing behavior |
| `chore:`  | Maintenance tasks, documentation, CI updates, etc.    |

#### Writing Guidelines

* **Title:** Use **imperative mood** (e.g., “add”, “fix”, “update”).
* **Body:**

  * Use **present tense** and **complete sentences** with explicit subjects (e.g., “This commit adds…”).
  * Focus on **what** and **why**, not how.
  * Keep the title under **50 characters** if possible.
  * Separate the title and body with a blank line.

#### Examples

```bash
chore: update README.md

This commit adds more related works and improves section formatting.
```

```bash
feat: add proxy server example

This commit implements a minimal TCP proxy for demonstration purposes.
```

#### References

* [freeCodeCamp: How to Write Better Git Commit Messages](https://www.freecodecamp.org/news/how-to-write-better-git-commit-messages/)
* [cbea.ms: How to Write a Git Commit Message](https://cbea.ms/git-commit/)

### Documentation

* Update `README.md` for user-facing changes (`feat:`)
* Add comments for complex algorithms

### Review Process

1. At least one maintainer must approve.
2. Address review feedback if requested.
3. The PR will be merged after approval.

---

是否要我幫你加上 Markdown 自動生成目錄標籤（例如 `[[toc]]` 或 `<details>` 可摺疊版）？
那樣在 GitHub 或 MkDocs 都會更方便瀏覽。
