#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <vector>

#include "depipe.hpp"

using namespace boost::asio;
using ip::tcp;
io_context io;

// Arguments
std::string proxy_host;
std::string ctrl_port;
u_short proxy_port;
std::string target_host = "127.0.0.1";
std::string target_port;

using ConnectHandler =
    std::function<void(const boost::system::error_code &, const tcp::endpoint)>;

void async_resolve_and_connect(tcp::socket &socket, const std::string &host,
                               const std::string &svc, ConnectHandler handler) {
    auto resolver = std::make_shared<tcp::resolver>(io);
    resolver->async_resolve(
        tcp::v4(), host, svc,
        [handler, &socket, resolver](const boost::system::error_code & ec_resolve,
    tcp::resolver::results_type results) {
        if (ec_resolve) {
            return handler(ec_resolve, tcp::endpoint{});
        }

        // 解析成功，繼續非同步連線
        boost::asio::async_connect(socket, results, handler);
    });
}

class Session : public std::enable_shared_from_this<Session> {
  public:
    explicit Session(u_short _agent_port)
        : agent_port(std::to_string(_agent_port)),
          proxy(io),
          target(io) {}

    void do_accept() {
        auto self(shared_from_this());
        std::cout << "Connection creating" << std::endl;
        async_resolve_and_connect(
            proxy, proxy_host, agent_port,
        [this, self](const boost::system::error_code & ec, const tcp::endpoint) {
            if (ec) {
                std::cout << "Proxy connection failed" << std::endl;
                return;
            }

            async_resolve_and_connect(
                target, target_host, target_port,
                [this, self](const boost::system::error_code & ec,
            const tcp::endpoint) {
                if (ec) {
                    std::cout << "Target connection failed" << std::endl;
                    return;
                }

                std::cout << "Connection created" << std::endl;
                auto piper = std::make_shared<depipe>(std::move(proxy),
                                                      std::move(target));
                piper->start();
            });
        });
    }

  private:
    std::string agent_port;
    tcp::socket proxy;
    tcp::socket target;
};

class Agent : public std::enable_shared_from_this<Agent> {
  public:
    Agent() : control(io), retry_timer(io) {}

    void do_request() {
        auto self(shared_from_this());
        async_resolve_and_connect(
            control, proxy_host, ctrl_port,
        [this, self](const boost::system::error_code & ec, const tcp::endpoint) {
            if (ec) {
                std::cout << ec.what() << std::endl;
                std::cout << "Proxy server not found" << std::endl;
                do_retry();
                return;
            }

            async_write(
                control, buffer(&proxy_port, 2),
            [this, self](const boost::system::error_code & ec, size_t size) {
                if (ec || size != 2) {
                    std::cout << "Proxy failed" << std::endl;
                    return;
                }

                std::cout << "Proxy at " << proxy_host << ":" << proxy_port
                          << std::endl;
                do_handle_connection();
            });
        });
    }

  private:
    void do_handle_connection() {
        auto self(shared_from_this());
        // looply wait for new connection from proxy server
        async_read(control, buffer(&random_port, 2),
        [this, self](const boost::system::error_code & ec, size_t size) {
            if (ec || size != 2) {
                std::cout << "Lose connection\n";
                do_retry();
                return;
            }

            std::make_shared<Session>(random_port)->do_accept();
            do_handle_connection();
        });
    }

    void do_retry() {
        auto self(shared_from_this());
        control.close();
        retry_timer.expires_after(std::chrono::seconds(3));
        std::cout << "Retry after 3 seconds\n";
        retry_timer.async_wait([this, self](const boost::system::error_code & ec) {
            if (ec) {
                return;
            }

            do_request();
        });
    }

  private:
    u_short random_port;
    tcp::socket control;
    boost::asio::steady_timer retry_timer;
};

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);

    // 使用 std::getline 配合分隔符號來提取每個部分
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

int main(int argc, const char *argv[]) {
    try {
        char *ptr = std::getenv("PROXY_HOST");

        if (ptr == nullptr) {
            throw std::invalid_argument("");
        }

        std::string proxy_server_link(ptr);
        size_t delimiter = proxy_server_link.find(':');

        if (delimiter == std::string::npos) {
            throw std::invalid_argument("");
        }

        proxy_host = proxy_server_link.substr(0, delimiter);
        ctrl_port = proxy_server_link.substr(delimiter + 1);
    } catch (...) {
        std::cerr << "Environment variable PROXY_HOST should follow the format "
                  "<proxy_host>:<ctrl_port>\n";
        return 1;
    }

    try {
        if (argc != 2) {
            throw std::invalid_argument("");
        }

        auto temp = split(argv[1], ':');
        proxy_port = std::stoi(temp[0]);

        switch (temp.size()) {
        case 2:
            target_port = temp[1];
            break;

        case 3:
            target_host = temp[1];
            target_port = temp[2];
            break;

        default:
            throw std::invalid_argument("");
        }
    } catch (...) {
        std::cerr << "Usage: expose <proxy_port>:[<target_host>:]<target_port>";
        return 1;
    }

    try {
        signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait(
        [](const boost::system::error_code & ec, int signal_number) {
            if (!ec) {
                io.stop();
            }
        });
        std::make_shared<Agent>()->do_request();
        std::vector<std::thread> threads;

        for (int i = 0; i < 4; ++i)
            threads.emplace_back([]() {
            io.run();
        });

        for (auto &t : threads)
            t.join();
    } catch (std::exception &e) {
        std::cerr << "error: " << e.what() << std::endl;
    }

    return 0;
}
