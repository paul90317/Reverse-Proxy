
# Reverse Proxy

A reverse proxy allows you to expose a local server that is located behind a NAT or firewall to the Internet [[1]](https://github.com/fatedier/frp). It currently supports TCP-based protocols, enabling requests to be forwarded to internal services via domain name or IP address.


## Highlights

* A C++ implementation of [frp](https://github.com/fatedier/frp), so no Go runtime installation is required.
* Unlike [ngrok](https://ngrok.com/), the proxy server can be deployed anywhere, which helps reduce propagation delay compared to connecting through ngrok’s global service.

## How It Works

This app does not directly forward traffic to your upstream services using IP addresses. Instead, you run a small piece of software alongside your service, called an *exposer*. The exposer establishes TCP connections to the proxy server. When traffic reaches your endpoints at the proxy server, it is transmitted to the exposer through these connections, and finally forwarded to your upstream service [[2]](https://ngrok.com/docs/how-ngrok-works/).


![img1.png](img1.png)。

## Setup and Usage

### 1️⃣ Install dependencies

#### Linux (Ubuntu/Debian)

```bash
sudo apt update

# Minimal Boost for your C++ project
sudo apt install -y g++ make libboost-system-dev libboost-thread-dev

# Static analysis & formatting tools
sudo apt install -y cppcheck clang-format
```

---
好 ✅ 我幫你把 README 裡的 **Build 教學段落**整理成跨平台版本：

---

## 🚀 Build Your Project

### 1️⃣ Configure

```bash
# 建立 git hooks
git config core.hooksPath .githooks

# 在專案根目錄建立 build/ 資料夾
mkdir build && cd build

# 產生編譯設定
cmake ..
```

---

### 2️⃣ Build

#### 🔹 Linux / macOS

```bash
# 編譯所有執行檔
cmake --build .
```

#### 🔹 Windows (MSVC)

```powershell
# 編譯 Release 版
cmake --build . --config Release

# 或編譯 Debug 版
cmake --build . --config Debug
```

---

### 3️⃣ Run the Proxy Server

```bash
./proxy_server 5000
```

---

### 4️⃣ Run the Exposer

```bash
export PROXY_HOST=<your_proxy_server_ip>:5000
./expose 80:80
```

---

### 5️⃣ Run static analysis & formatting

```bash
# Format all C++ files according to .clang-format
clang-format -i *.cpp *.hpp

# Run static analysis
cppcheck --enable=all --inconclusive --std=c++11 --quiet *.cpp *.hpp
```



