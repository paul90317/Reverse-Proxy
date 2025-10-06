#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <utility>

using namespace boost::asio;
using ip::tcp;

// Assuming 'io' is defined in the application's main file
extern io_context io;

/**
 * @class ssocket
 * @brief 使用組合 (Composition) 模式封裝 tcp::socket，並為其分配一個專屬 Strand，
 * 確保所有異步操作的 Completion Handlers 都是線程安全的。
 */
class ssocket
{
private:
    // 1. 核心變動：使用組合模式，將 tcp::socket 作為私有成員
    tcp::socket socket_;

    // 2. 專門為這個 socket 服務的執行序列 (Strand)
    io_context::strand strand_;

public:
    // 構造函數 A (新增): 接受一個 io_context，用於創建全新的 socket。
    // 這允許直接在給定的執行環境中初始化 ssocket。
    ssocket(io_context &io_context)
        : socket_(io_context),
          strand_(io_context)
    {
    }

    // 構造函數 B (重要): 接受一個已配置的 tcp::socket (例如從 acceptor 或 resolver 得到)，
    // 並將其狀態移動到內部成員中。
    ssocket(tcp::socket &&other_socket)
        // 構造 socket_ 成員，使用移動語義接管 other_socket 的狀態
        : socket_(std::move(other_socket)),
          // 修復: strand 必須使用 socket 所在的執行器 (executor) 來構造，確保在同一 io_context 上運行
          strand_(io)
    {
    }

    // 禁用複製，確保只有一個 ssocket 實例管理該 Strand
    ssocket(const ssocket &) = delete;
    ssocket &operator=(const ssocket &) = delete;

    /**
     * @brief 線程安全地啟動異步讀取操作 (async_read_some)。
     */
    template <typename MutableBufferSequence, typename ReadHandler>
    void async_read_some(const MutableBufferSequence &buffers, ReadHandler &&handler)
    {
        // 核心邏輯：將處理程序綁定到 strand_，確保該 socket 的所有操作序列化。
        socket_.async_read_some(buffers,
                                boost::asio::bind_executor(strand_, std::forward<ReadHandler>(handler)));
    }

    /**
     * @brief 線程安全地啟動異步讀取操作（使用 async_read free function），保證讀取完所有要求的數據。 (新增)
     */
    template <typename MutableBufferSequence, typename ReadHandler>
    void async_read(const MutableBufferSequence &buffers, ReadHandler &&handler)
    {
        // 使用 async_read 自由函數，將處理程序綁定到 strand_。
        boost::asio::async_read(socket_, buffers,
                                boost::asio::bind_executor(strand_, std::forward<ReadHandler>(handler)));
    }

    /**
     * @brief 線程安全地啟動異步寫入部分數據操作 (async_write_some)。
     */
    template <typename ConstBufferSequence, typename WriteHandler>
    void async_write_some(const ConstBufferSequence &buffers, WriteHandler &&handler)
    {
        // 核心邏輯：將處理程序綁定到 strand_，確保該 socket 的所有操作序列化。
        socket_.async_write_some(buffers,
                                 boost::asio::bind_executor(strand_, std::forward<WriteHandler>(handler)));
    }

    /**
     * @brief 線程安全地啟動異步寫入操作（使用 async_write free function），保證寫完所有數據。
     */
    template <typename ConstBufferSequence, typename WriteHandler>
    void async_write(const ConstBufferSequence &buffers, WriteHandler &&handler)
    {
        // 使用 async_write 自由函數，將處理程序綁定到 strand_。
        boost::asio::async_write(socket_, buffers,
                                 boost::asio::bind_executor(strand_, std::forward<WriteHandler>(handler)));
    }

    // --- 實用功能：暴露底層 socket 的必要方法 ---

    // 暴露 close 方法
    void close()
    {
        boost::system::error_code ec;
        if (socket_.is_open())
        {
            socket_.close(ec);
        }
    }

    // 暴露 endpoint 資訊
    ip::tcp::endpoint remote_endpoint() const
    {
        boost::system::error_code ec;
        // boost::asio 的 remote_endpoint 函式可能會拋出異常，這裡使用 error_code 版本
        return socket_.remote_endpoint(ec);
    }

    bool is_open() const
    {
        return socket_.is_open();
    }

    // 暴露本地 endpoint 資訊
    ip::tcp::endpoint local_endpoint() const
    {
        boost::system::error_code ec;
        return socket_.local_endpoint(ec);
    }
};
