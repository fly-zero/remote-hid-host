#include "communication.h"

#include <memory>
#include <WS2tcpip.h>

#include <string>
#include <system_error>

#include "common.h"

namespace remote_hid
{

struct communication::init
{
    init();

    ~init();

    init(const init&) = delete;
    void operator=(const init&) = delete;
    init(init&&) = delete;
    void operator=(init&&) = delete;

    static init s_init;
};

communication::init communication::init::s_init;

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
    // Create a socket
    auto const sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "Failed to create socket");
    }

    // spit address
    auto const [host, port] = split_string(addr, ':');
    if (host.empty() || port.empty())
    {
        closesocket(sock);
        throw exception::invalid_argument("Invalid address: '%.*s'", static_cast<int>(addr.size()), addr.data());
    }

    // copy host to null-terminated string
    auto const host_deleter = [](char* p) { _freea(p); };
    std::unique_ptr<char, decltype(host_deleter)> const host_str{ static_cast<char*>(_malloca(host.size() + 1)), host_deleter };
    *std::copy(host.begin(), host.end(), host_str.get()) = '\0';

    // setup address
    sockaddr_in tmp{};
    tmp.sin_family = AF_INET;
    tmp.sin_port = htons(static_cast<uint16_t>(std::stoi(port.data())));
    if (inet_pton(AF_INET, host_str.get(), &tmp.sin_addr) != 1)
    {
        auto const err = WSAGetLastError();
        closesocket(sock);
        throw std::system_error(err, std::system_category(), "Invalid address");
    }

    // bind socket
    if (bind(sock, reinterpret_cast<sockaddr*>(&tmp), sizeof(tmp)) == SOCKET_ERROR)
    {
        auto const err = WSAGetLastError();
        closesocket(sock);
        throw std::system_error(err, std::system_category(), "bind failed");
    }

    // listen
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        auto const err = WSAGetLastError();
        closesocket(sock);
        throw std::system_error(err, std::system_category(), "listen failed");
    }

    // setup member
    sock_ = sock;
}

communication::~communication()
{
    // cleanup
    if (sock_ != INVALID_SOCKET)
    {
        closesocket(sock_);
    }
}

inline communication::init::init()
{
    // Initialize Winsock
    WSADATA wsa_data;
    if (auto const err = WSAStartup(MAKEWORD(2, 2), &wsa_data); err != 0)
    {
        throw std::system_error(err, std::system_category(), "WSAStartup failed");
    }
}

inline communication::init::~init()
{
    WSACleanup();
}



}
