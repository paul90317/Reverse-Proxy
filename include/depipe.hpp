#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <array>
#include "ssocket.hpp" // 引入 Strand-Safe Socket

using namespace boost::asio;
using ip::tcp;

extern io_context io;

// Define a reasonable buffer size for efficiency
constexpr size_t BUF_SIZE = 4096;

/**
 * @class depipe
 * @brief 使用 Strand-Safe Sockets (ssocket) 在兩個端點之間異步傳輸數據。
 *
 * 每個 ssocket 內部都有自己的 Strand，因此 forward 和 backward 兩個管道
 * 可以完全並行執行，解決了原始設計中的單一 Strand 瓶頸。
 */
class depipe : public std::enable_shared_from_this<depipe> {
  public:
    // 構造函數：接受兩個已建立的 tcp::socket，並將它們移動到 ssocket 成員中。
    // ssocket 的 Move 構造函數會自動為每個 socket 創建一個專屬的 Strand。
    depipe(tcp::socket _src, tcp::socket _dest)
        : src(std::move(_src)), dest(std::move(_dest)) {
    }

    void start() {
        // 啟動兩個獨立的管道。它們在 ssocket 內部 Strand 的保護下並行運行。
        pipe_forward();
        pipe_backward();
    }

  private:
    // *** 核心變動：使用 ssocket 替代 tcp::socket。Strand 管理已內建於 ssocket 中。 ***
    ssocket src;
    ssocket dest;
    // io_context::strand strand; // <-- 不再需要！

    // A shared helper for closing both sockets
    void close_sockets() {
        // 呼叫 ssocket 內的安全關閉方法
        src.close();
        dest.close();
    }

    // Data flow from src to dest
    void pipe_forward() {
        auto self(shared_from_this());
        auto buffer_ptr = std::make_shared<std::array<char, BUF_SIZE>>();

        // 讀取操作：調用 ssocket::async_read_some。
        // 其 Completion Handler 會自動在 src.ssocket 內部的 Strand 上執行。
        src.async_read_some(buffer(*buffer_ptr),
        [this, self, buffer_ptr](const boost::system::error_code & ec_read, size_t bytes_read) {
            // 檢查讀取錯誤或 EOF
            if (ec_read == boost::asio::error::eof || ec_read) {
                close_sockets();
                return;
            }

            // 寫入操作：調用 dest.ssocket::async_write。
            // 由於此 Handler (read_handler) 在 src.strand 上執行，
            // 而 Write Handler (write_handler) 將在 dest.strand 上執行，
            // 這是兩個獨立的 Strand，不會互相等待。
            dest.async_write(buffer(*buffer_ptr, bytes_read),
            [this, self, buffer_ptr](const boost::system::error_code & ec_write, size_t) {
                // 檢查寫入錯誤
                if (ec_write) {
                    close_sockets();
                    return;
                }

                // 繼續管道傳輸 (下一輪 Read 將重新回到 src.strand 上)
                pipe_forward();
            });
        });
    }

    // Data flow from dest to src
    void pipe_backward() {
        auto self(shared_from_this());
        auto buffer_ptr = std::make_shared<std::array<char, BUF_SIZE>>();

        // 讀取操作：Completion Handler 將在 dest.ssocket 內部的 Strand 上執行。
        dest.async_read_some(buffer(*buffer_ptr),
        [this, self, buffer_ptr](const boost::system::error_code & ec_read, size_t bytes_read) {
            // 檢查讀取錯誤或 EOF
            if (ec_read == boost::asio::error::eof || ec_read) {
                close_sockets();
                return;
            }

            // 寫入操作：Completion Handler 將在 src.ssocket 內部的 Strand 上執行。
            // 這同樣允許與 pipe_forward 並行。
            src.async_write(buffer(*buffer_ptr, bytes_read),
            [this, self, buffer_ptr](const boost::system::error_code & ec_write, size_t) {
                // 檢查寫入錯誤
                if (ec_write) {
                    close_sockets();
                    return;
                }

                // 繼續管道傳輸 (下一輪 Read 將重新回到 dest.strand 上)
                pipe_backward();
            });
        });
    }
};
