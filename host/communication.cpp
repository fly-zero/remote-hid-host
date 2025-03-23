#include "communication.h"

#include <WS2tcpip.h>

#include <string>

#include "common.h"

namespace remote_hid
{

const char* str_wsa_error(int const err)
{
    // format error string
    static char buffer[4096];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
        nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, _countof(buffer), nullptr);
    return buffer;
}

std::tuple<std::string_view, std::string_view> split_string(std::string_view str, char delim)
{
    auto const pos = str.find(delim);
    if (pos == std::string_view::npos)
    {
        return { str, {} };
    }

    return { str.substr(0, pos), str.substr(pos + 1) };
}

communication::communication(std::string_view addr)
{
    // Initialize Winsock
    WSADATA wsaData;
    auto const result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        throw exception::runtime_error("WSAStartup failed: %s", str_wsa_error(result));
    }

    // Create a socket
    auto const sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        throw exception::runtime_error("socket failed: %s", str_wsa_error(result));
    }

    // spit address
    auto const [host, port] = split_string(addr, ':');
    if (host.empty() || port.empty())
    {
        closesocket(sock);
        throw exception::invalid_argument("Invalid address");
    }

    // copy host to null-terminated string
    auto const host_str = static_cast<char *>(_malloca(host.size() + 1));
    *std::copy(host.begin(), host.end(), host_str) = '\0';

    // setup address
    sockaddr_in tmp{};
    tmp.sin_family = AF_INET;
    tmp.sin_port = htons(static_cast<uint16_t>(std::stoi(port.data())));
    if (inet_pton(AF_INET, host_str, &tmp.sin_addr) != 1)
    {
        closesocket(sock);
        throw exception::invalid_argument("Invalid address");
    }

    // bind socket
    if (bind(sock, reinterpret_cast<sockaddr*>(&tmp), sizeof(tmp)) == SOCKET_ERROR)
    {
        auto const err = WSAGetLastError();
        closesocket(sock);
        throw exception::runtime_error("bind failed: %s", str_wsa_error(err));
    }

    // listen
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        auto const err = WSAGetLastError();
        closesocket(sock);
        throw exception::runtime_error("listen failed: %s", str_wsa_error(err));
    }

    // setup member
    sock_ = sock;
}

communication::~communication()
{
    // cleanup
    closesocket(sock_);
    WSACleanup();
}

}
