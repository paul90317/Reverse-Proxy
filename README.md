
# Reverse Proxy

A reverse proxy allows you to expose a local server that is located behind a NAT or firewall to the Internet [[1]](https://github.com/fatedier/frp). It currently supports TCP-based protocols—such as **HTTP/HTTPS** for web services, **database connections** (MySQL, PostgreSQL), and **Minecraft** game servers—enabling requests to be forwarded to internal services via domain name or IP address.

## Existing Solutions
* [nginx](https://nginx.org/) ("engine x") is an HTTP web server, reverse proxy, content cache, load balancer, and TCP/UDP proxy server.
  However, it becomes useless when the local port cannot be directly exposed.
* Unlike [ngrok](https://ngrok.com/), this solution allows the proxy server to be deployed anywhere, reducing propagation delay compared to ngrok's centralized global service.
* [`ssh -R`](https://linux.die.net/man/1/ssh) forwards the given port on the remote (server) host to the given host and port on the local side.
  However, if sshd doesn't set [GatewayPorts](https://linux.die.net/man/5/sshd_config), only loopback addresses can be bound at the remote host, thus prohibiting other hosts from connecting.
* The bind operation in [SOCKS](https://www.openssh.com/txt/socks4.protocol) allows binding a local port to the proxy,
  but it requires rebinding the port each time a new client connects.
* This solution is a C++ implementation of [frp](https://github.com/fatedier/frp), so you don't need to install or run the Go runtime.

## Highlights

* **C++ Implementation**: A native C++ port of frp, eliminating the dependency on Go runtime environment and reducing deployment complexity.
* **Flexible Deployment**: Proxy servers can be deployed anywhere, avoiding the propagation delays inherent in ngrok's centralized infrastructure.
* **No GatewayPorts Configuration**: Solves the remote accessibility limitations of `ssh -R` without requiring sshd configuration changes.
* **Persistent Port Binding**: Unlike SOCKS protocol, it maintains stable port bindings without requiring reconnection for each new client.

## How It Works

This app does not directly forward traffic to your upstream services using IP addresses. Instead, you run a small piece of software alongside your service, called an *exposer*. The exposer establishes TCP connections to the proxy server. When traffic reaches your endpoints at the proxy server, it is transmitted to the exposer through these connections, and finally forwarded to your upstream service [[2]](https://ngrok.com/docs/how-ngrok-works/).


![sequence diamgram](docs/assets/sequence%20diagram.png)


## Build, Run and Contributing

Core build and contribution instructions live in the project's contribution guide. See:

- License: [`LICENSE`](./LICENSE) (short: MIT)
- Contribution & developer setup: [`CONTRIBUTING.md`](./CONTRIBUTING.md)

If you just want a quick start to build and run the main binaries locally, go to the Releases page, choose a version, download the executable ZIP file for your operating system, then extract it and run:

```bash
# For Proxy
./proxy_server 5000

# For Exposer
export PROXY_HOST=<your_proxy_server_ip>:5000
./expose 80:80
```

