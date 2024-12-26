#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
using namespace boost::asio;
using ip::tcp;

class depipe : public std::enable_shared_from_this<depipe>
{
public:
    depipe(tcp::socket _src, tcp::socket _dest)
        : src(std::move(_src)), dest(std::move(_dest))
    {
    }

    void pipe()
    {
        auto self(shared_from_this());
        src.async_read_some(buffer(buf), [this, self](const boost::system::error_code &ec, size_t bytes_read)
                            {
            if (ec) {
                dest.close();
                src.close();
                return;
            }
            async_write(dest, buffer(buf, bytes_read), [this, self](const boost::system::error_code &ec, size_t)
            {
                if (ec) {
                    dest.close();
                    src.close();
                    return;
                }
                pipe();
            }); });
    }
    void rpipe()
    {
        auto self(shared_from_this());
        dest.async_read_some(buffer(rbuf), [this, self](const boost::system::error_code &ec, size_t bytes_read)
                             {
            if (ec) {
                dest.close();
                src.close();
                return;
            }
            async_write(src, buffer(rbuf, bytes_read), [this, self](const boost::system::error_code &ec, size_t)
            {
                if (ec) {
                    dest.close();
                    src.close();
                    return;
                }
                rpipe();
            }); });
    }

private:
    std::array<char, 1> buf;
    std::array<char, 1> rbuf;
    tcp::socket src;
    tcp::socket dest;
};
