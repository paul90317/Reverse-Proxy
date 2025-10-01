#include <boost/asio.hpp>
#include <iostream>

#include "depipe.hpp"

using namespace boost::asio;
using ip::tcp;
io_context context;
tcp::resolver resolver(context);

// Arguments
std::string proxy_server_ip;
u_short ctrl_port;
u_short proxy_port;
u_short target_port;

tcp::endpoint target_endpoint;

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(u_short _agent_port)
      : agent_port(_agent_port), proxy(context), target(context) {}

  void do_bridge() {
    auto self(shared_from_this());
    std::cout << "Connection creating" << std::endl;
    resolver.async_resolve(
        tcp::v4(), proxy_server_ip, std::to_string(agent_port),
        [this, self](const boost::system::error_code &ec,
                     tcp::resolver::results_type results) {
          if (ec) {
            std::cout << "Proxy server not found" << std::endl;
            return;
          }
          async_connect(
              proxy, results,
              [this, self](const boost::system::error_code &ec,
                           const tcp::endpoint) {
                if (ec) {
                  std::cout << "Proxy connection failed" << std::endl;
                  return;
                }
                target.async_connect(
                    target_endpoint,
                    [this, self](const boost::system::error_code &ec) {
                      if (ec) {
                        std::cout << "Target connection failed" << std::endl;
                        return;
                      }
                      std::cout << "Connection created" << std::endl;
                      auto piper = std::make_shared<depipe>(std::move(proxy),
                                                            std::move(target));
                      piper->pipe();
                      piper->rpipe();
                    });
              });
        });
  }

 private:
  u_short agent_port;
  tcp::socket proxy;
  tcp::socket target;
};

class Agent : public std::enable_shared_from_this<Agent> {
 public:
  Agent(tcp::socket _control) : control(std::move(_control)) {}

  void do_request() {
    auto self(shared_from_this());
    resolver.async_resolve(
        tcp::v4(), proxy_server_ip, std::to_string(ctrl_port),
        [this, self](const boost::system::error_code &ec,
                     tcp::resolver::results_type results) {
          if (ec) {
            std::cout << "Proxy server not found" << std::endl;
            return;
          }
          async_connect(control, results,
                        [this, self](const boost::system::error_code &ec,
                                     const tcp::endpoint) {
                          if (ec) {
                            std::cout << "Proxy server not found" << std::endl;
                            return;
                          }

                          memcpy(buf.data(), &proxy_port, 2);
                          std::swap(buf[0], buf[1]);
                          async_write(
                              control, buffer(buf),
                              [this, self](const boost::system::error_code &ec,
                                           size_t bytes_read) {
                                if (ec) {
                                  std::cout << "Proxy failed" << std::endl;
                                  return;
                                }
                                std::cout << "Proxy at " << proxy_server_ip
                                          << ":" << proxy_port << std::endl;
                                do_handle_connection();
                              });
                        });
        });
  }

 private:
  void do_handle_connection() {
    auto self(shared_from_this());
    // looply wait for new connection from proxy server
    async_read(
        control, buffer(buf),
        [this, self](const boost::system::error_code &ec, size_t bytes_read) {
          if (ec) {
            std::cout << "Lose connection\n";
            return;
          }
          std::swap(buf[0], buf[1]);
          u_short port = *(u_short *)buf.data();
          std::cout << "New random port at " << port << std::endl;
          std::make_shared<Session>(port)->do_bridge();
          do_handle_connection();
        });
  }

 private:
  std::array<char, 2> buf;
  tcp::socket control;
};

int main(int argc, char *argv[]) {
  char *ptr = std::getenv("PROXY_HOST");
  if (ptr == nullptr) {
    std::cerr << "Environment variable PROXY_HOST not set\n";
    return 1;
  }
  try {
    std::string proxy_server_link(ptr);
    size_t delimiter = proxy_server_link.find(':');
    if (delimiter == std::string::npos) {
      std::cerr << "Environment variable PROXY_HOST should follow the format "
                   "<server_ip>:<server_port>\n";
      return 1;
    }
    proxy_server_ip = proxy_server_link.substr(0, delimiter);
    ctrl_port = std::stoi(proxy_server_link.substr(delimiter + 1));
  } catch (...) {
    std::cerr << "Environment variable PROXY_HOST should follow the format "
                 "<server_ip>:<server_port>\n";
    return 1;
  }

  if (argc != 2) {
    std::cerr << "Usage: expose <proxy_port>:<target_port>\n";
    return 1;
  }

  try {
    std::string proxy_target = argv[1];
    size_t delimiter = proxy_target.find(':');
    if (delimiter == std::string::npos) {
      std::cerr << "Usage: expose <proxy_port>:<target_port>\n";
      return 1;
    }
    proxy_port = std::stoi(proxy_target.substr(0, delimiter));
    target_port = std::stoi(proxy_target.substr(delimiter + 1));
  } catch (...) {
    std::cerr << "Usage: expose <proxy_port>:<target_port>\n";
    return 1;
  }

  while (true) {
    try {
      target_endpoint =
          tcp::endpoint(ip::make_address("127.0.0.1"), target_port);
      std::make_shared<Agent>(tcp::socket(context))->do_request();
      context.run();
      context.restart();
      std::this_thread::sleep_for(std::chrono::seconds(3));  // 暫停 3 秒
    } catch (std::exception &e) {
      std::cerr << "Agent error: " << e.what() << std::endl;
    }
  }

  return 0;
}
