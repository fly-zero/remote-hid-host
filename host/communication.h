#pragma once

#include <WinSock2.h>

#include <string_view>

namespace remote_hid
{

class communication
{
    struct init;

public:
    explicit communication(std::string_view addr);

    ~communication();

    communication(communication&& other) noexcept;

    communication& operator=(communication&& other) noexcept;

    communication(communication const&) = delete;

    communication& operator=(communication const&) = delete;

private:
    // socket handle
    SOCKET sock_{INVALID_SOCKET};
};

inline communication::communication(communication&& other) noexcept
    : sock_(std::exchange(other.sock_, INVALID_SOCKET))
{
}

inline communication& communication::operator=(communication&& other) noexcept
{
    if (this != &other)
    {
        // close the current socket
        if (sock_ != INVALID_SOCKET)
        {
            closesocket(sock_);
        }

        // move the socket handle
        sock_ = std::exchange(other.sock_, INVALID_SOCKET);
    }

    return *this;
}

}
