#pragma once

#include <WinSock2.h>

#include <string_view>

namespace remote_hid
{

class communication
{
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

}
