
# Reverse Proxy

A reverse proxy allows you to expose a local server that is located behind a NAT or firewall to the Internet [[1]](https://github.com/fatedier/frp). It currently supports TCP-based protocols, enabling requests to be forwarded to internal services via domain name or IP address.

## Highlights

* A C++ implementation of [frp](https://github.com/fatedier/frp), so you don’t need to install or run the Go runtime.
* Unlike [ngrok](https://ngrok.com/), the proxy server can be deployed anywhere, reducing propagation delay compared to ngrok’s centralized global service.
* [nginx](https://nginx.org/) (“engine x”) is an HTTP web server, reverse proxy, content cache, load balancer, and TCP/UDP proxy server.
  However, it becomes useless when the local port cannot be directly exposed.
* The bind operation in [SOCKS](https://www.openssh.com/txt/socks4.protocol) allows binding a local port to the proxy,
  but it requires rebinding the port each time a new client connects.


## How It Works

This app does not directly forward traffic to your upstream services using IP addresses. Instead, you run a small piece of software alongside your service, called an *exposer*. The exposer establishes TCP connections to the proxy server. When traffic reaches your endpoints at the proxy server, it is transmitted to the exposer through these connections, and finally forwarded to your upstream service [[2]](https://ngrok.com/docs/how-ngrok-works/).


![img1.png](img1.png)。


## 🚀 Build and Run

### 1️⃣ Install Dependencies

#### Linux (Ubuntu/Debian)

```bash
sudo apt update

# Minimal Boost for your C++ project
sudo apt install -y g++ make libboost-system-dev libboost-thread-dev

# Static analysis & formatting tools
sudo apt install -y cppcheck clang-format
```

---

### 2️⃣ Configure the Build

```bash
# Create a build/ folder at the project root
mkdir build && cd build

# Generate the build configuration
cmake ..
```

---

### 3️⃣ Compile

#### 🔹 Linux / macOS

```bash
# Build all executables
cmake --build .
```

#### 🔹 Windows (MSVC)

```powershell
# Build for Release
cmake --build . --config Release

# Build for Debug
cmake --build . --config Debug
```

---

### 4️⃣ Run the Proxy Server

```bash
./proxy_server 5000
```

---

### 5️⃣ Run the Exposer

```bash
export PROXY_HOST=<your_proxy_server_ip>:5000
./expose 80:80
```

