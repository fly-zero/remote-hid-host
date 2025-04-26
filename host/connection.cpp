#include "connection.h"

#include <mswsock.h>

#include <iostream>

#include "server.h"

namespace remote_hid
{

void connection::on_io_complete(DWORD bytes_transferred) { resume(); }

void connection::run()
{
    if (!server_.get_event_dispatch().register_listener(*this))
    {
        return;
    }

    while (true)
    {
        // receive socket
        char       buffer[1024];
        auto const n = recv(buffer, sizeof buffer);
        if (n == 0) break;
        (std::cout << "received " << n << " bytes: ").write(buffer, n) << std::endl;
    }

    server_.close(*this);
}

long connection::recv(char *buffer, unsigned size) const
{
    // receive data
    WSABUF     wsa_buf{size, buffer};
    DWORD      flags = 0;
    OVERLAPPED overlapped{};
    auto const res = WSARecv(get_socket(), &wsa_buf, 1, nullptr, &flags, &overlapped, nullptr);
    if (res == SOCKET_ERROR)
    {
        if (WSAGetLastError() != ERROR_IO_PENDING)
        {
            throw std::system_error(WSAGetLastError(), std::system_category(), "WSARecv");
        }
    }

    yield();

    DWORD bytes;
    if (!WSAGetOverlappedResult(get_socket(), &overlapped, &bytes, FALSE, &flags))
    {
        throw std::system_error(
            WSAGetLastError(), std::system_category(), "WSAGetOverlappedResult");
    }

    return static_cast<long>(bytes);
}

}  // namespace remote_hid