#include <boost/asio.hpp>
#include <iostream>
#include <thread>

#include "depipe.hpp"

using namespace boost::asio;
using ip::tcp;
io_context context;

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(tcp::socket _client)
      : client(std::move(_client)), agent_acceptor(context) {}
  void do_connect_agent(tcp::socket *control) {
    auto self(shared_from_this());

    tcp::endpoint endpoint(tcp::v4(), 0);
    agent_acceptor.open(endpoint.protocol());
    agent_acceptor.set_option(tcp::acceptor::reuse_address(true));
    agent_acceptor.bind(endpoint);
    agent_acceptor.listen();

    u_short port = agent_acceptor.local_endpoint().port();
    memcpy(buf.data(), &port, 2);
    std::cout << "New random port at " << port << std::endl;
    std::swap(buf[0], buf[1]);
    async_write(
        *control, buffer(buf),
        [this, self](const boost::system::error_code &ec, size_t size) {
          if (ec || size != 2) {
            return;
          }

          agent_acceptor.async_accept([this, self](boost::system::error_code ec,
                                                   tcp::socket agent) {
            if (ec) {
              return;
            }
            agent_acceptor.close();
            std::cout << "Connection created" << std::endl;
            auto piper =
                std::make_shared<depipe>(std::move(client), std::move(agent));
            piper->pipe();
            piper->rpipe();
          });
        });
  }

 private:
  std::array<char, 2> buf;
  tcp::socket client;
  tcp::acceptor agent_acceptor;
};

class Agent : public std::enable_shared_from_this<Agent> {
 public:
  Agent(tcp::socket _control) : control(std::move(_control)), proxy(context) {}
  void do_proxy() {
    auto self(shared_from_this());
    async_read(control, buffer(buf),
               [this, self](const boost::system::error_code &ec, size_t size) {
                 if (ec || size != 2) {
                   return;
                 }
                 std::swap(buf[0], buf[1]);
                 port = *reinterpret_cast<u_short *>(buf.data());
                 std::cout << "New proxy request the port " << port
                           << std::endl;
                 tcp::endpoint endpoint(tcp::v4(), port);
                 proxy.open(endpoint.protocol());
                 proxy.set_option(tcp::acceptor::reuse_address(true));
                 try {
                   proxy.bind(endpoint);
                 } catch (...) {
                   std::cerr << "Failed to bind to port " << port << std::endl;
                   return;
                 }
                 proxy.listen();
                 do_accept();
                 do_check_control();
               });
  }

 private:
  void do_accept() {
    auto self(shared_from_this());
    proxy.async_accept([this, self](boost::system::error_code ec,
                                    tcp::socket client) {
      if (ec) {
        std::cerr << "Proxy closed at " << port << std::endl;
        return;
      }
      std::cout << "New connection at " << proxy.local_endpoint().port()
                << std::endl;
      std::make_shared<Session>(std::move(client))->do_connect_agent(&control);
      do_accept();
    });
  }

  void do_check_control() {
    auto self(shared_from_this());
    control.async_read_some(
        buffer(buf), [this, self](boost::system::error_code ec, std::size_t) {
          // agent don't read the data from control
          // so when this handler executed, it must be an error
          proxy.close();
        });
  }

  std::array<char, 2> buf;
  tcp::socket control;
  tcp::acceptor proxy;
  u_short port;
};

class Server : public std::enable_shared_from_this<Server> {
 public:
  Server(u_short port) : acceptor(context) {
    tcp::endpoint endpoint(tcp::v4(), port);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();
  }
  void do_accept() {
    auto self(shared_from_this());
    acceptor.async_accept(
        [this, self](boost::system::error_code ec, tcp::socket agent) {
          if (ec) return;
          std::make_shared<Agent>(std::move(agent))->do_proxy();
          do_accept();
        });
  }

 private:
  tcp::acceptor acceptor;
};

int main(int argc, char *argv[]) {
  u_short control_port;
  if (argc != 2) {
    std::cerr << "Usage: proxy_server <control_port>\n";
    return 1;
  }
  try {
    control_port = std::stoi(argv[1]);
  } catch (...) {
    std::cerr << "Usage: proxy_server <control_port>\n";
    return 1;
  }
  try {
    std::make_shared<Server>(control_port)->do_accept();
    std::cout << "Server started on port " << control_port << std::endl;
    context.run();
  } catch (std::exception &e) {
    std::cerr << "Agent error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
