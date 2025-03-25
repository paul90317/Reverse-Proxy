# Reverse Proxy

A tool to map internal network server ports to external computers, allowing clients to access internal servers.

## How It Works

![img1.png](img1.png)

The process works in three stages:

1. **First Stage**: The proxy server listens on port 5000 for expose commands from the internal network.

2. **Second Stage**: The exposer connects to port 5000 and sends the required port to listen on (in this case, 80).

3. **Third Stage**: The client connects to the proxy server, and the proxy server:
   - Assigns a random available port based on the previous connection
   - Waits for the connection from the exposer
   - The exposer connects to both the local server and the proxy server
   - A TCP connection is established

## Setup and Usage

### Prepare Makefile

Choose the appropriate Makefile for your system:

```bash
# For Windows 11
cp win11/Makefile Makefile

# For Ubuntu
cp ubuntu/Makefile Makefile
```

### Running the Proxy Server

```bash
make
./proxy_server 5000
```

### Running the Exposer

```bash
make
export PROXY_HOST=<your_proxy_server_ip>:5000
./expose 80:80
```



